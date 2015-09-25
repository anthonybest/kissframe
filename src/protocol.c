#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <time.h>
#include <sys/time.h>


#include "fcs16.h"
#include "frame.h"


#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>

struct termios saved_termios;
int have_saved_termios = 0;

void setup_tty(int fd)
{
	struct termios tio;
	memset(&tio, 0, sizeof(tio));

	// Save settings.
	if(tcgetattr(fd, &saved_termios))
	{
		// error
		//log_msg("tcgetattr: %s", strerror(errno));
		perror("tcgetattr");                        
		//abort();
	}
	have_saved_termios = -1;

	tio = saved_termios;

	tio.c_iflag 	|= IGNBRK | ISTRIP | IGNPAR;
	tio.c_iflag 	&=  ~ISTRIP;
	tio.c_oflag		 = 0;
	tio.c_lflag		 = 0;
	tio.c_cc[VERASE] = 0;
	tio.c_cc[VKILL]  = 0;
	tio.c_cc[VMIN]	 = 0;
	tio.c_cc[VTIME]	 = 0;

	if(tcsetattr(fd, TCSANOW, &tio))
	{
		perror("tcsetattr");
		//log_msg("tcsetattr: '%s'", strerror(errno));
		//abort();
	}
} 

int openPort(const char * device)
{
	int fd;
	struct termios tio;

	//fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK);
	fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY);
	if(fd <= 0)
	{
		perror("open");
		exit(-1);
	}

	fcntl(fd, F_SETFL, FNDELAY);


	ioctl(fd, TIOCSCTTY, (void*)1);
	fcntl(fd, F_SETFL, 0);
	memset(&tio, 0, sizeof(tio));
	tcgetattr(fd, &tio);
	cfsetispeed(&tio, B57600);
	cfsetospeed(&tio, B57600);
	tio.c_cflag |= (CLOCAL | CREAD);
	tio.c_cflag &= ~PARENB;
	tio.c_cflag &= ~CSTOPB;
	tio.c_cflag &= ~CSIZE;
	tio.c_cflag |= CS8;


	//tio.c_cflag &= ~CNEW_RTSCTS;
	tio.c_cflag &= ~CRTSCTS;

	tio.c_iflag &= ~(IXON | IXOFF | IXANY);
	
    //tio.c_iflag |= (INPCK | ISTRIP);
	

	//tio.c_lflag &= ~(ICANON |ECHO | ECHOE);
	tio.c_lflag &= ~(ECHO | ECHOE);
	tio.c_oflag &= ~OPOST;
	tcsetattr(fd, TCSANOW, &tio);

	setup_tty(fd);

	//write(fd, "Hello\n", 6);

	return fd;
}




struct packetWindow
{
	unsigned char buffer[8][1024];
	bool ack[8];
	int seq;
};

bool dump = true;

void dumpHex( void * vdata, size_t size)
{
	unsigned char *data  = (unsigned char *)vdata;
	int i;

	if(!dump)
		return;
	for(i = 0; i < size; i += 16)
	{
		int j;
		for(j = 0; j < 16; j++)
		{
			if(i+j<size)
				printf("%02x ", data[i+j]);
			else
				printf("   ");
		}

		for(j = 0; j < 16; j++)
		{
			if(i+j<size)
				printf("%c", isprint(data[i+j]) ? data[i+j] : '.');
			else
				printf(" ");
		}
		printf("\n");
	}
	//return 1;
}

void sendPacket(int fd, unsigned char * buf, size_t len)
{
		unsigned char sendbuf[1024*3];
		int s;
		s = encode_frame(buf, sendbuf, len);

		static int noise = 0;
		if(noise++ == 7)
		{
			//sendbuf[0] = 'x';
			noise = 0;
		}

		printf("Send buffer:\n");
		dumpHex(sendbuf, s);
		write(fd, sendbuf, s);
		//write(fd, "GO\0p", 4);
		//printf("sent\n");
}

void sendAck(int fd, int port, int seq)
{
	unsigned char ackpack[] = { 0,0,0 };
	ackpack[0] = (uint8_t)port;
	//ackpack[1] = ( (seq & 0xf) << 4) | (0x4) | ( 0x3);
	ackpack[2] = seq & 0xff;
	ackpack[1] = 0x4 | 0x3;
	sendPacket(fd, ackpack, sizeof(ackpack));
	//printf("sent ack %i\n", seq);
	//dumpHex(ackpack, sizeof(ackpack));
}

struct packet
{
	int port;
	int seq;
	bool bit0;
	bool bit1;
	bool ack;
	bool bit3;
	bool bit4;
	unsigned char * data;
	size_t len;
};


#define FRAME_BIT0	1
#define FRAME_BIT1	2
#define FRAME_BIT2	3

#define frameHeaderSize  3
struct frameHeader
{
	uint8_t address;
	uint8_t flags;
	uint8_t seq;
};

size_t encodePacket(uint8_t address, uint8_t seq, uint8_t * data, size_t len, uint8_t *buffer)
{
	//size_t size;
	buffer[0] = address;
	buffer[1] = 0x3;
	buffer[2] = seq;
	memcpy(buffer+3, data, len);
	return 3 + len;
}

bool decodePacket(frame_t * frame, struct packet * packet)
{
	if(frame->data_len < 3)
		return false;

	//printf("decode frame\n");
	//dumpHex(frame->data, frame->data_len);
	
	packet->port = frame->data[0];
	packet->seq = (frame->data[2]) & 0xff;
	//packet->seq = 99;
	packet->bit0 = (frame->data[1] & 0x1) ? true : false;
	packet->bit1 = (frame->data[1] & 0x2) ? true : false;
	packet->ack = (frame->data[1] & 0x4) ? true : false;
	packet->bit3 = (frame->data[1] & 0x8) ? true : false;
	packet->bit4 = (frame->data[1] & 0xe) ? true : false;
	packet->data = frame->data + 3;
	packet->len = frame->data_len - 3;
	if(packet->ack)
		printf("decoded ack %i\n", packet->seq);
	return true;

}





void serverLoop()
{
	/*
	//dump = false;
    if((mkfifo("/tmp/framein", S_IRUSR | S_IWUSR) < 0 ) && (EEXIST != errno))
	{
		fprintf(stderr, "Error opening in\n");
		exit(-1);
	}
	printf("pt %i\n", __LINE__);

	if((mkfifo("/tmp/frameout", S_IRUSR | S_IWUSR) < 0) && (EEXIST != errno))
	{
		fprintf(stderr, "Error opening out\n");
		exit(-1);
	}
	printf("pt %i\n", __LINE__);

	int readfd = open("/tmp/framein", O_RDONLY | O_NONBLOCK);
	if(readfd<0)
	{
		perror("open");
		exit(-1);
	}
	printf("pt %i\n", __LINE__);
	int writefd = open("/tmp/frameout", O_WRONLY);
	if(writefd<0)
	{
		perror("open");
		exit(-1);
	}
	printf("pt %i\n", __LINE__);
	*/
	int writefd, readfd;


    writefd = readfd = openPort("/dev/tty.usbserial-FTBNM9OA");

	unsigned char buf[100];

	frame_t frame;
	unsigned char buffer[1024];
	frame_setbuffer(&frame, buffer, sizeof(buffer));
	frame_reset(&frame);

	printf("Serverloop stars\n");
	int nextSeq = 0;


	do
	{

		//printf("About to read\n");
		int r = read(readfd, buf, 1);
		//printf("done read\n");
		//printf("<%02x>\n", buf[0]);
		if(r == 1)
		{
			//printf( "sgot %02x\n", buf[0]);
			if(frame_input(&frame, buf[0]))
			{
				struct packet pkt;
				if(decodePacket(&frame, &pkt) && pkt.port == 0)
				{
					if(pkt.bit0 && pkt.bit1)
					{
						//printf(" nextSeq = %i pkt->seq = %i\n" , nextSeq, pkt.seq);
						if(nextSeq == pkt.seq)
						{
							nextSeq = (nextSeq+1)%255;
							//printf("packet\n");
							dumpHex(pkt.data, pkt.len);
						}
						//else{ printf("miss\n"); }
							//dumpHex(pkt.data, pkt.len);
						//else
							//dumpHex(frame.data, frame.data_len);

					}
					//else
						//dumpHex(frame.data, frame.data_len);
				}
				//else
					//dumpHex(frame.data, frame.data_len);

				sendAck(writefd, pkt.port, pkt.seq);
				//printf("sent ack %i \n", pkt.seq );


				frame_reset(&frame);
			}
		}
		else
		{
			//printf("Server r = %i\n", r);
			//exit(-1);
		}

				/*
		//printf("pt %i\n", __LINE__);

		printf("Server:\n");
		dumpHex(buf, r);
		printf("\n");

		*/
		//r = write(writefd, "R", 1);
	} while(1);
			



}

//struct packetWindow clientWindow;


void clientLoop()
{
	/*
	int writefd = open("/tmp/framein", O_WRONLY);
	printf("cli pt %i\n", __LINE__);
	int readfd = open("/tmp/frameout", O_RDONLY | O_NONBLOCK);
	printf("cli pt %i\n", __LINE__);
	*/

	int writefd, readfd;

    writefd = readfd = openPort("/dev/tty.KeySerial1");

	//sleep(6000);

	//write(writefd, "\xc0""ELLO", 5);


	//return;

	int seq = 0;

	int ctr = 0;

	frame_t frame;
	unsigned char framebuffer[1024];
	frame_setbuffer(&frame, framebuffer, sizeof(framebuffer));
	frame_reset(&frame);
	printf("client loop start\n");
	//clientWindow.seq = 0;

	//int i;
	//for(i = 0; i < 8; i++)
		//clientWindow.ack[i] = true;

	unsigned char data[] = { 'H', 'e', 'l', 'l', 'o' , '0'};
	unsigned char lastpacket[1024];
	unsigned char lastpacketSize = 0;
	time_t lastpacketTimestamp = 0;
	int lastpacketLoop = 0;

	//printf("sizeof(time_t) %li\n", sizeof(time_t));
	int lastpacketSeq = 0;
	int lastpacketRetrans = 0;

	int loopCount = 0;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	time_t epoc = tv.tv_sec;

	while(1)
	{
		loopCount++;

		//if ( (lastpacketSize > 0) && loopCount > lastpacketLoop+4/*&& time(NULL) >= (lastpacketTimestamp + 1)*/)
		if ( (lastpacketSize > 0) && loopCount > lastpacketLoop+4 && time(NULL) >= (lastpacketTimestamp + 2))
		{
			printf("resend packet\n");
			sendPacket(writefd, lastpacket, lastpacketSize);
			lastpacketTimestamp = time(NULL);
			lastpacketLoop = loopCount;
			lastpacketRetrans++;
			if(lastpacketRetrans > 9)
			{
				printf("rety timeout\n");
				exit(-1);
			}
		}
		else if (lastpacketSize == 0)
		{
			//sleep(1);

			if(ctr > 3)
				ctr = 0;
			if(ctr == 0)
			{
				/*
				data[1] = seq & 0xff;
				data[2] = 0x3;
				data[8] = '0' + (seq % 10);

				{
					seq++;

					//printf("send packet\n");
					lastpacketSize = sizeof(data);
					memcpy(lastpacket, data, sizeof(data));
					//lastpacketTimestamp = time(NULL);
					lastpacketLoop = loopCount;
					lastpacketSeq = data[1] & 0xff;

					sendPacket(writefd, data, sizeof(data));
				}
				*/
				uint8_t buffer[sizeof(data) + 3];
				data[5] = '0' + (seq %10);
				//printf("seq = %i\n", seq);
				size_t s = encodePacket(0, seq, data, sizeof(data), buffer);
				lastpacketSize = s;
				memcpy(lastpacket, buffer, s);
				lastpacketLoop = loopCount;
				lastpacketSeq = seq;
				seq = (seq+1)%256;
				sendPacket(writefd, buffer, s);
				lastpacketTimestamp = time(NULL);
				//dumpHex(buffer, s);
			}

			else
			{
				//printf("client idle %i\n", ctr);
			}
			ctr++;
		}


		char buf[100];

		int len;
		int r = 0;

		do
		{
			//printf("readx\n");
			len = read(readfd, buf, 1);
			//printf("len = %i\n", len);
			//if(len == -1)
				//perror("read");
			if(len == 1)
			{
				//printf("(%02x)", (unsigned char)buf[0]);
				//printf("frame.data_len = %i\n", (int)frame.data_len);
				if(frame_input(&frame, buf[0]))
				{
                    struct packet pkt;
					//printf("Client: ");
					//dumpHex(frame.data, frame.data_len);
					if(decodePacket(&frame, &pkt) && pkt.port == 0)
					{
						if(pkt.bit0 && pkt.bit1)
						{
							if( (pkt.seq == lastpacketSeq) && lastpacketLoop)
							{
								//printf("got ack %i\n", pkt.seq);
								lastpacketSize = lastpacketSeq = lastpacketLoop = lastpacketRetrans = 0;
							}
							else 
							{
								printf("pkt.seq = %i lastpacketSeq = %i\n", pkt.seq, lastpacketSeq);
								printf("seq mismatch\n");
							}
						}
						else 
							//dumpHex(frame.data, frame.data_len);
							printf("packet mismatch\n");

					}
					else 
						//dumpHex(frame.data, frame.data_len);
						printf("decode failed\n");
					frame_reset(&frame);
				}
				//printf("<>");
			}
		} while(len == 1);
		//printf("liip\n");
		if(r) {}
		//printf("sleep\n");

		uint32_t s;

		gettimeofday(&tv, NULL);
		s = (tv.tv_sec - epoc) * 1000;
		s += (tv.tv_usec/1000);
		//printf("%u\n", s);

		usleep(10000);

		
		gettimeofday(&tv, NULL);
		s = (tv.tv_sec - epoc) * 1000;
		s += (tv.tv_usec/1000);
		//printf("%u\n", s);

	};





}
