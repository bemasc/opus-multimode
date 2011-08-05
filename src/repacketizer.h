#include "opus.h"
#define RP_MAXSIZE 65535

typedef struct FrameCell FrameCell;
typedef struct FrameBuffer FrameBuffer;
typedef struct Repacketizer Repacketizer;

struct FrameCell {
	FrameCell *next;
	unsigned char toc;
	int size;
	unsigned char *data;
	int duration;
};

struct FrameBuffer {
	FrameCell *first;
	FrameCell *last;
	int numframes;
	int total_duration;
};

struct Repacketizer {
	int samples_sent;
	FrameBuffer buffer;
};

OPUS_EXPORT void rp_init(Repacketizer* st);
OPUS_EXPORT int rp_load_packet(Repacketizer *st, const unsigned char *data, int len);
OPUS_EXPORT int rp_get_packet(Repacketizer *st, int interval, unsigned char *data);
