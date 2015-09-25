#include <stdbool.h>
typedef struct
{
	unsigned char * data;
	size_t data_alloc;
	size_t data_len;
	
	uint16_t fcs;
	unsigned char fcs_bytes[2];
	size_t fcs_pos;
	bool escape_flag;
} frame_t;

void frame_reset(frame_t * frame);
void frame_setbuffer(frame_t * frame, unsigned char * buffer, size_t len);
bool frame_input(frame_t * frame, unsigned char c/*, int (*fp)(unsigned char *, size_t, void *), void * vptr*/);
void frame_reset(frame_t * frame);

int encode_frame(unsigned char *s, unsigned char *d, int len);
int decode_frame(const uint8_t * rawframe, size_t size, uint8_t * buffer, size_t bufferlen);

