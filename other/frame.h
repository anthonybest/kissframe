// 8k buffer is max frame size
#define BUFFER_SIZE	(8*1024)

#define FEND	0xc0
#define FESC	0xdb
#define TFEND	0xdc
#define TFESC	0xdd

typedef struct 
{
	uint8_t buffer[BUFFER_SIZE];
	uint16_t fcs;
	size_t length;
	bool escape;
} Frame_t;

void resetFrame(Frame_t * frame);
bool putByte(Frame_t * frame, uint8_t c);

uint8_t * escape_byte(const uint8_t  c, uint8_t * buf);
size_t encodeFrame(const uint8_t * msg, size_t len, uint8_t * buffer);

