
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "fcs16.h"
#include "frame.h"

/*
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

*/

void resetFrame(Frame_t * frame)
{
	frame->fcs = INITFCS16;
	frame->length = 0;
	frame->escape = false;
}

bool putByte(Frame_t * frame, uint8_t c)
{
	if(FEND == c)
	{
		return true;
	}

	if(frame->escape)
	{
		switch(c)
		{
			case TFEND:
				c = FEND;
				break;

			case TFESC:
				c = FESC;
				break;

			default:
				resetFrame(frame);
				return false;
		}
	}
	else
	{
		if(FESC == c)
		{
			frame->escape = true;
			return false;
		}
	}

	if(frame->length >= sizeof(frame->buffer))
	{
		resetFrame(frame);
		return false;
	}

	frame->buffer[frame->length++] = c;
	frame->fcs = fcs16(frame->fcs, c);

	return false;
}


uint8_t * escape_byte(const uint8_t  c, uint8_t * buf)
{
	if(FEND == c)
	{
		*(buf++) = FESC;
		*(buf++) = TFEND;
	}
	else if (FESC == c)
	{
		*(buf++) = FESC;
		*(buf++) = TFESC;
	}
	else
	{
		*(buf++) = c;
	}
	return buf;
}

// ideally buffer should be (len*2) + 6 for worst case. For every char you know is not FESC or FEND subtract 2
// 4 bytes is for worst case of FCS16 having both bytes escaped, and, 2 more for FEND
size_t encodeFrame(const uint8_t * msg, size_t len, uint8_t * buffer)
{
	uint16_t fcs = INITFCS16;
	uint8_t c;
	uint8_t * p = buffer;

	*p++ = FEND;	// Flush buffer;
	while(len--)
	{
		c = *msg++;
		fcs = fcs16(fcs, c);
		p = escape_byte(c, p);
	}
	fcs ^= 0xffff;
	// FCS16 stored Little Ending
	p = escape_byte(fcs & 0xff, p);
	p = escape_byte((fcs>>8) & 0xff, p);
	*p++ = FEND;

	return p-buffer;
}


