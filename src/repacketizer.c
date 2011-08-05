#include "repacketizer.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"

#define MAX_PACKET (1275)

static int parse_size(const unsigned char *data, int len, short *size)
{
	if (len<1)
	{
		*size = -1;
		return -1;
	} else if (data[0]<252)
	{
		*size = data[0];
		return 1;
	} else if (len<2)
	{
		*size = -1;
		return -1;
	} else {
		*size = 4*data[1] + data[0];
		return 2;
	}
}

static void rp_fill_framecell(FrameCell *fc, unsigned char toc, const unsigned char *data, int len)
{
	fc->next = NULL;
	fc->toc = toc;
	fc->size = len;
	fc->data = malloc(len);
	memcpy(fc->data, data, len);
	fc->duration = opus_packet_get_samples_per_frame(&toc, 48000);
}

static void rp_clear_framecell(FrameCell *fc)
{
	free(fc->data);
}

static int rp_load_frame(Repacketizer *st, unsigned char toc, const unsigned char *data, int len)
{
	FrameCell *prevlast = st->buffer.last;
	st->buffer.last = malloc(sizeof(FrameCell));
	if (prevlast != NULL)
		prevlast->next = st->buffer.last;
	if (st->buffer.first == NULL)
		st->buffer.first = st->buffer.last;
	rp_fill_framecell(st->buffer.last, toc, data, len);
	
	st->buffer.total_duration += st->buffer.last->duration;
	st->buffer.numframes++;
	return st->buffer.last->duration; /* Don't know how to fail yet */
}

int rp_load_packet(Repacketizer *st, const unsigned char *data, int len)
{
	int i, bytes, nb_samples;
	int count;
	unsigned char ch, toc;
	int multimode=0;
	int frame_size;
	/* 48 x 2.5 ms = 120 ms */
	short size[48];
	if (len==0 || data==NULL)
	    return OPUS_CORRUPTED_DATA; /* FIXME: handle lost packets */
	else if (len<0)
		return OPUS_BAD_ARG;
	frame_size = opus_packet_get_samples_per_frame(data, 48000);
	toc = *data++;
	len--;
	switch (toc&0x3)
	{
	/* One frame */
	case 0:
		count=1;
		size[0] = len;
		break;
		/* Two CBR frames */
	case 1:
		count=2;
		if (len&0x1)
			return OPUS_CORRUPTED_DATA;
		size[0] = size[1] = len/2;
		break;
		/* Two VBR frames */
	case 2:
		count = 2;
		bytes = parse_size(data, len, size);
		len -= bytes;
		if (size[0]<0 || size[0] > len)
			return OPUS_CORRUPTED_DATA;
		data += bytes;
		size[1] = len-size[0];
		break;
		/* Multiple CBR/VBR frames (from 0 to 120 ms) */
	case 3:
		if (len<1)
			return OPUS_CORRUPTED_DATA;
		/* Number of frames encoded in bits 0 to 5 */
		ch = *data++;
		if (ch == 49)  /* 49 is a reserved value indicating multimode */
		{ 
			multimode = 1;
			len--;
			ch = *data++; /* In multimode the next byte is the real ch */
		}
		count = ch&0x3F;
		if (count <= 0 || (!multimode && frame_size*count*25 > 3*48000))
		    return OPUS_CORRUPTED_DATA;
		len--;
		/* Padding flag is bit 6 */
		if (ch&0x40)
		{
			int padding=0;
			int p;
			do {
				if (len<=0)
					return OPUS_CORRUPTED_DATA;
				p = *data++;
				len--;
				padding += p==255 ? 254: p;
			} while (p==255);
			len -= padding;
		}
		/* Account for the extra count-1 TOC bytes in multimode */
		if (multimode)
			len -= count-1;
		if (len<0)
			return OPUS_CORRUPTED_DATA;
		/* VBR flag is bit 7 */
		if (ch&0x80)
		{
			/* VBR case */
			int last_size=len;
			for (i=0;i<count-1;i++)
			{
				bytes = parse_size(data, len, size+i);
				len -= bytes;
				if (size[i]<0 || size[i] > len)
					return OPUS_CORRUPTED_DATA;
				data += bytes;
				last_size -= bytes+size[i];
			}
			if (last_size<0)
				return OPUS_CORRUPTED_DATA;
			size[count-1]=last_size;
		} else {
			/* CBR case */
			int sz = len/count;
			if (sz*count!=len)
				return OPUS_CORRUPTED_DATA;
			for (i=0;i<count;i++)
				size[i] = sz;
		}
		break;
	}
	/* Because it's not encoded explicitly, it's possible the size of the
	    last packet (or all the packets, for the CBR case) is larger than
	    1275.
	   Reject them here.*/
	if (size[count-1] > MAX_PACKET)
		return OPUS_CORRUPTED_DATA;
	nb_samples=0;
	for (i=0;i<count;i++)
	{
		int ret;
		/* Multimode only affects the second and subsequent frames */
		if (multimode && i > 0)
		{
			if (*data&0x3 != 0) /* Subpackets in multimode must have Code 0 */
				return OPUS_CORRUPTED_DATA;
			/* Add 1 byte to size, to account for the TOC byte */
			ret = rp_load_packet(st, data, size[i]+1);
			/* Move the pointer past the TOC byte */
			data++; 
		} else
		{
			ret = rp_load_frame(st, toc, data, size[i]);
		}
		if (ret<0)
			return ret;
		data += size[i];
		nb_samples += ret;
	}
	return nb_samples;
}

int encode_size(short size, unsigned char *data)
{
	if (size <= 251)
	{
		*data = size;
		return 1;
	} else {
		size -= 252;
		*data++ = 252 + (size % 4);
		*data = size/4;
		return 2;
	}
}

int rp_get_packet(Repacketizer *st, int interval, unsigned char *data)
{
	/* Returns number of bytes written.
	   interval is specified in samples at 48k.
	   *data must have at least 61250 bytes allocated. */
	int duration = st->samples_sent;
	int count = 0, i;
	short size[48];
	unsigned char *start = data;
	FrameCell *fc = st->buffer.first;
	if (interval > 48*120)
		interval = 48*120;
	if (st->samples_sent >= interval)
	{
		/* Reset delay accumulator; there must have been an interval change */
		st->samples_sent = 0;
	}
	if (st->samples_sent + st->buffer.total_duration < interval)
		return 0;
	while (duration < interval && duration + fc->duration <= 48*120)
	{
		duration += fc->duration;
		size[count] = fc->size;
		fc = fc->next;
		count += 1;
	}
	fc = st->buffer.first;
	*data++ = fc->toc | 0x3; /* Force code 3 */
	*data++ = 49; /* Multi-mode */
	*data++ = count | 0x80; /* Number of frames, VBR, no padding */
	for(i=0;i<count-1;i++)
		data += encode_size(size[i], data);
	for(i=0;i<count;i++)
	{
		if (i > 0)
			*data++ = fc->toc & (~0x3); /* Force code 0 */
		memcpy(data, fc->data, fc->size);
		data += fc->size;

		st->buffer.total_duration -= fc->duration;
		st->buffer.first = fc->next;
		rp_clear_framecell(fc);
		free(fc);
		st->buffer.numframes--;

		fc = st->buffer.first;
	}
	if (!st->buffer.first)
		st->buffer.last = NULL;
	return data-start;
}

void rp_init(Repacketizer* st)
{
	st->samples_sent = 0;
	st->buffer.total_duration = 0;
	st->buffer.first = NULL;
	st->buffer.last = NULL;
	st->buffer.numframes = 0;
	st->buffer.total_duration = 0;
	
}

static void print_usage( char* argv[] ) 
{
    fprintf(stderr, "Usage: %s <input> <output>\n\n", argv[0]);
    fprintf(stderr, "options:\n" );
    fprintf(stderr, "-interval <number in ms>: sets the repacketization interval (default=60)\n" );
}

static void int_to_char(opus_uint32 i, unsigned char ch[4])
{
    ch[0] = i>>24;
    ch[1] = (i>>16)&0xFF;
    ch[2] = (i>>8)&0xFF;
    ch[3] = i&0xFF;
}

static opus_uint32 char_to_int(unsigned char ch[4])
{
    return ((opus_uint32)ch[0]<<24) | ((opus_uint32)ch[1]<<16)
         | ((opus_uint32)ch[2]<< 8) |  (opus_uint32)ch[3];
}

int main(int argc, char *argv[])
{
   int err;
   char *inFile, *outFile;
   FILE *fin, *fout;
   int args;
   int len[2];
   int interval;
   unsigned char *data[2];
   int sampling_rate = 48000;
   int stop=0;
   int tot_read=0, tot_written=0;
   int enc_final_range;
   Repacketizer rp;

   rp_init(&rp);

   if (argc < 3 )
   {
      print_usage( argv );
      return 1;
   }

   /* defaults: */
   interval = 3*sampling_rate/50;

   args = 1;
   while( args < argc - 2 ) {
       /* process command line options */
        if( strcmp( argv[ args ], "-interval" ) == 0 ) {
            interval = (int)(sampling_rate * atof(argv[ args + 1 ]));
            if (interval < sampling_rate/400 || interval > 3*sampling_rate/25) {
                fprintf(stderr, "Unsupported interval: %s ms. Supported are 2.5-120.\n", argv[ args + 1 ]);
                return 1;
            }
            args += 2;
        } else {
            printf( "Error: unrecognized setting: %s\n\n", argv[ args ] );
            print_usage( argv );
            return 1;
        }
   }

   inFile = argv[argc-2];
   fin = fopen(inFile, "rb");
   if (!fin)
   {
      fprintf (stderr, "Could not open input file %s\n", argv[argc-2]);
      return 1;
   }
   outFile = argv[argc-1];
   fout = fopen(outFile, "wb+");
   if (!fout)
   {
      fprintf (stderr, "Could not open output file %s\n", argv[argc-1]);
      return 1;
   }

   fprintf(stderr, "Repacketizing on interval %d.\n", interval);

   data[0] = (unsigned char*)calloc(RP_MAXSIZE, sizeof(char));
   data[1] = (unsigned char*)calloc(RP_MAXSIZE, sizeof(char));
   while (!stop)
   {
      /* Load a packet */
      unsigned char ch[4];
      err = fread(ch, 1, 4, fin);
      len[0] = char_to_int(ch);
      if (len[0]>RP_MAXSIZE || len[0]<0)
      {
    	  fprintf(stderr, "Invalid payload length\n");
    	  break;
      }
      err = fread(ch, 1, 4, fin);
      enc_final_range = char_to_int(ch);
      err = fread(data[0], 1, len[0], fin);
      tot_read += len[0];
      if (feof(fin))
          break;

      if (rp_load_packet(&rp, data[0], len[0]) < 0)
      {
          fprintf(stderr, "Error loading packet\n");
          break;
      }
      
      len[1] = rp_get_packet(&rp, interval, data[1]);
      if (len[1] > 0)
      {
          unsigned char int_field[4];
          int_to_char(len[1], int_field);
          fwrite(int_field, 1, 4, fout);
          int_to_char(enc_final_range*0, int_field); /* Don't actually handle range coder state */
          fwrite(int_field, 1, 4, fout);
          fwrite(data[1], 1, len[1], fout);
          tot_written += len[1];
      }
   }

   free(data[0]);
   free(data[1]);
   fclose(fin);
   fclose(fout);
   return 0;
}
