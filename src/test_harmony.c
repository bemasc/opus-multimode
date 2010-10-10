/* Copyright (c) 2007-2008 CSIRO
   Copyright (c) 2007-2009 Xiph.Org Foundation
   Written by Jean-Marc Valin */
/*
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "harmony.h"


#define MAX_PACKET 1024

int main(int argc, char *argv[])
{
   int err;
   char *inFile, *outFile;
   FILE *fin, *fout;
   HarmonyEncoder *enc;
   HarmonyDecoder *dec;
   int len;
   int frame_size, channels;
   int bytes_per_packet;
   unsigned char data[MAX_PACKET];
   int rate;
   int loss = 0;
   int count = 0;
   int skip;
   int stop=0;
   int vbr=0;
   int tot_read=0, tot_written=0;
   short *in, *out;
   int mode=MODE_HYBRID;
   double bits=0;
   if (argc != 9 && argc != 8 && argc != 7)
   {
      fprintf (stderr, "Usage: test_harmony <rate (kHz)> <channels> <frame size> "
               " <bytes per packet>  [<VBR rate (kb/s)>] [<packet loss rate>] "
               "<input> <output>\n");
      return 1;
   }

   rate = atoi(argv[1]);
   channels = atoi(argv[2]);
   frame_size = atoi(argv[3]);

   bytes_per_packet = atoi(argv[4]);

   if (argc >= 8)
       vbr = atoi(argv[5]);
   if (argc >= 9)
       loss = atoi(argv[6]);

   if (bytes_per_packet < 0 || bytes_per_packet > MAX_PACKET)
   {
      fprintf (stderr, "bytes per packet must be between 0 and %d\n",
                        MAX_PACKET);
      return 1;
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

   enc = harmony_encoder_create(rate);
   dec = harmony_decoder_create(rate);

   mode = MODE_HYBRID;
   harmony_encoder_ctl(enc, HARMONY_SET_BANDWIDTH(BANDWIDTH_FULLBAND));
   harmony_encoder_ctl(enc, HARMONY_SET_MODE(mode));

   if (vbr)
       harmony_encoder_ctl(enc, HARMONY_SET_VBR_RATE(vbr));

   skip = 5*rate/1000 + 10;

   in = (short*)malloc(frame_size*channels*sizeof(short));
   out = (short*)malloc(frame_size*channels*sizeof(short));
   while (!stop)
   {
      int write_samples;
      err = fread(in, sizeof(short), frame_size*channels, fin);
      tot_read += err;
      if (err < frame_size*channels)
      {
          int i;
          for (i=err;i<frame_size*channels;i++)
              in[i] = 0;
      }
      len = harmony_encode(enc, in, frame_size, data, bytes_per_packet);
      if (len <= 0)
      {
         fprintf (stderr, "harmony_encode() returned %d\n", len);
         return 1;
      }
      bits += len*8;
      harmony_decode(dec, rand()%100<loss ? NULL : data, len, out, frame_size);
      count++;
      tot_written += (frame_size-skip)*channels;
      write_samples = frame_size;
      if (tot_written > tot_read && skip==0)
      {
          write_samples -= (tot_written-tot_read)/channels;
          stop = 1;
      }
      fwrite(out+skip, sizeof(short), (write_samples-skip)*channels, fout);
      skip = 0;
   }
   fprintf (stderr, "average bit-rate: %f kb/s\n", bits*rate/(frame_size*(double)count));
   harmony_encoder_destroy(enc);
   harmony_decoder_destroy(dec);
   fclose(fin);
   fclose(fout);
   free(in);
   free(out);
   return 0;
}