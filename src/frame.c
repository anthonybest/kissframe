#include <stdio.h>
#include <stdbool.h>
#include "fcs16.h"
#include "frame.h"

// Frame characters
// See RFC1055, AX.25 KISS
// Values were not chosen arbitrarily, don't change them
#define FEND 0xc0
#define FESC 0xdb
#define FTEND 0xdc
#define FTESC 0xdd


int encode_frame(unsigned char *s, unsigned char *d, int len)
{
	unsigned char * p = d;
	unsigned char c;
	uint16_t fcs = INITFCS16;


	*p++ = FEND;

	while(len-- > 0)
	{
		c = *s++;
		fcs = fcs16_byte(fcs, c);
		switch(c)
		{
			case FEND:
				*p++ = FESC;
				*p++ = FTEND;
				break;

			case FESC:
				*p++ = FESC;
				*p++ = FTESC;
				break;

			default:
				*p++ = c;
				break;
		}
	}
	// TODO: fix this problem.
	unsigned char fcsc[2];
	fcs16_compliment(fcs, fcsc);
	if(fcsc[0] == FEND)
	{
		*p++ = FESC;
		*p++ = FTEND;
	}else if(fcsc[0] == FESC)
	{
		*p++ = FESC;
		*p++ = FTESC;

	}else
		*p++ = fcsc[0];
	if(fcsc[1] == FEND)
	{
		*p++ = FESC;
		*p++ = FTEND;
	}else if(fcsc[1] == FESC)
	{
		*p++ = FESC;
		*p++ = FTESC;

	}else
		*p++ = fcsc[1];

	//p += 2;
	*p++ = FEND;


	return p - d;
}



void frame_reset(frame_t * frame)
{
	frame->data_len = 0;
	frame->fcs = INITFCS16;
	frame->fcs_pos = 0;
	frame->escape_flag = false;
}

void frame_setbuffer(frame_t * frame, unsigned char * buffer, size_t len)
{
	frame_reset(frame);
	frame->data = buffer;
	frame->data_alloc = len;
}


bool frame_input(frame_t * frame, unsigned char c/*, int (*fp)(unsigned char *, size_t, void *), void * vptr*/)
{
	if(FEND == c)
	{
		//int r = 0;

		if(frame->data_len && frame->fcs == GOODFCS16)
		{
			return true;
			/*
			if(fp)
			{
				r = fp(frame->data, frame->data_len, vptr);
			}
			*/
		}
		else
		{
			if(frame->data_len)
				printf("Frame CRC error\n");
		}
		//printf("fcs = %02x\n", frame->fcs);
		frame_reset(frame);
		return false;
		/*
		return r;
		*/
	}
	//printf("byte\n");
	if( FESC == c)
	{
		frame->escape_flag = true;
		return false;
	}

	if(frame->escape_flag)
	{
		if(FTEND == c)
			c = FEND;
		else if(FTESC == c)
			c = FESC;
		else
		{
			//printf("unexpexted %02x\n", c);
			frame_reset(frame);
			return false;
		}
		frame->escape_flag = false;
	}

	//printf("fcs = %04x\n", fcs16_byte(frame->fcs, c));
	//printf("fcs_pos = %i\n", (int)frame->fcs_pos);
	if(frame->fcs_pos == 2)
	{
		if(frame->data_len >= frame->data_alloc)
		{
			frame_reset(frame);
			return false;
		}
		frame->data[frame->data_len++] = frame->fcs_bytes[0];
		frame->fcs_bytes[0] = frame->fcs_bytes[1];
		frame->fcs_pos--;
	}
	frame->fcs_bytes[frame->fcs_pos++] = c;
	frame->fcs = fcs16_byte(frame->fcs, c);

	return false;
}

int decode_frame(const uint8_t * rawframe, size_t size, uint8_t * buffer, size_t bufferlen)
{
	frame_t frame;
	int i;

	frame_reset(&frame);
	frame_setbuffer(&frame, buffer, bufferlen);

	for(i = 0; i < size; i++)
	{
		//printf("...%02x\n", rawframe[i]);
		if(frame_input(&frame, rawframe[i]))
		{
			//printf("data len = %i\n", (int)frame.data_len);
			return frame.data_len;
		}
	}

	return 0;



}

