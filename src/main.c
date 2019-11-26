#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include "fcs16.h"
#include "frame.h"
#include "testdata.h"

int dumpData(unsigned char * data, size_t size, void * vptr)
{
	int i;
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
	/*


	int i;
	for(i = 0; i < size; i++)
	{
		printf("%02x%c", data[i], ( (i+1)%16 ? ' ' : '\n'));
	}
	if(0 == (i%16))
		printf("\n");

		*/
	return 1;
}


bool test_fcs16_sanity()
{
	int     i;
	uint16_t fcs;
	unsigned char check[2];
	const unsigned char __fcs16_test1[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	printf(" %s: ", __func__);

	fcs = INITFCS16;
	for (i = 0; i < sizeof(__fcs16_test1); i++)
		fcs = fcs16_byte(fcs, __fcs16_test1[i]);
	if (fcs != 0xc3e9)
	{
		printf("FAIL! ");
		printf("fcs %04x ", fcs);
		printf("Expected 0xc3e9\n");
		return false;
	}
    if(!fcs16_compliment(fcs, check))
	{
		printf("FAIL! fcs16_compliment() failed\n");
		return false;
	}
	if( (check[0] != 0x16) || (check[1] != 0x3c) )
	{
		printf("FAIL! compliment failed expecting %02x %02x: Got  %02x %02x, \n", 0x16, 0x3c, check[0], check[1]);
		return false;
	}

	printf("PASS!\n");
	return true;
}

bool test_fcs16_values(unsigned char * data, size_t len, uint16_t c_fcs, unsigned char c_cfcs0, unsigned char c_cfcs1)
{
	int i;
	uint16_t fcs = INITFCS16;
	unsigned char check[2];

	printf(" %s: ", __func__);
	/*
	for(i = 0; i < len; i++)
		printf("%02x%c", data[i], (((len+1) % 16)? ' ' : '\n'));
	printf("\n");
	*/

	for(i = 0; i < len; i++)
		fcs = fcs16_byte(fcs, data[i]);
	//printf("FCS expecting %04x: Got %04x: ", c_fcs, fcs);
	if(fcs != c_fcs)
	{
		//printf("Expecting FCS %04x, got %04x\n", c_fcs, fcs);
		printf("Failed\n");
		return false;
	}
	//printf("Pass\n");

	//printf("Compliment: Expecting %02x %02x: ", c_cfcs0, c_cfcs1);

	if(!fcs16_compliment(fcs, check))
	{
		printf("fcs16_compliment() failed\n");
		return false;
	}

	//printf("Got %02x %02x: ", check[0], check[1]);

	if( (check[0] != c_cfcs0) || (check[1] != c_cfcs1) )
	{
		printf("Failed\n");
		//printf("compliment failed expecting %02x %02x: Got  %02x %02x, \n", c_cfcs0, c_cfcs1, check[0], check[1]);
		return false;
	}
	printf("Pass\n");

	return true;
}

unsigned char __fcs16_test2[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

bool test_fcs16_stream()
{
	int i;
	uint16_t fcs = INITFCS16;

	printf(" %s: ", __func__);

	for(i = 0; i < sizeof(__fcs16_test2); i++)
	{
		fcs = fcs16_byte(fcs, __fcs16_test2[i]);
	}
	if(fcs != 0xc3e9)
	{
		printf("Failed\n");
		return false;
	}
	fcs = fcs16_byte(fcs, 0x16);
	fcs = fcs16_byte(fcs, 0x3c);
	if(fcs != GOODFCS16)
	{
		printf("Failed 2\n");
		return false;
	}
	printf("PASS.\n");
	return true;
}


/*
void test_fcs16_a(char * data)
{
	int i;
	uint16_t fcs = INITFCS16;
	for(i = 0; data[i]; i++)
	{
		fcs = fcs16_byte(fcs, data[i]);
		printf(" '%x' %04x\n", data[i], fcs);
	}
}
*/

bool test_fcs16_allvalues()
{
	//uint8_t buffer[1024];
	uint16_t fcs = INITFCS16;
	int i;
	printf(" %s: ", __func__);

	

	for(i = 0; i < (sizeof(fcs16seq)/sizeof(fcs16seq[0])); i++)
	{
		fcs = fcs16_byte(fcs, (i&0xff));
		if(fcs != fcs16seq[i])
		{
			printf("FAIL! on Seq %i expecting %04x got %04x\n", i, fcs16seq[i], fcs);
			return false;
		}
	}

	printf("Pass\n");

	return true;
}


unsigned char chkdata2[][5] = { 
	{ 0x00, 0x0f, 0x3a,		0xdb, 0xdd},
	{ 0x00, 0x1e, 0x32,		0xdb, 0xdc},
	{ 0x00, 0x20, 0x64,		0xc0, 0xdd},
	{ 0x00, 0x2e, 0xd7,		0xdd, 0xdd},
	{ 0x00, 0x31, 0x6c,		0xc0, 0xdc},
	{ 0x00, 0x3f, 0xdf,		0xdd, 0xdc},
	{ 0x00, 0x46, 0x54,		0xc0, 0xdb},
	{ 0x00, 0x48, 0xe7,		0xdd, 0xdb},
	{ 0x00, 0x69, 0x0a,		0xdb, 0xdb},
	{ 0x00, 0x6b, 0x6f,		0xdc, 0xc0},
	{ 0x00, 0xa6, 0x86,		0xdc, 0xdd},
	{ 0x00, 0xb7, 0x8e,		0xdc, 0xdc},
	{ 0x00, 0xc0, 0xb6,		0xdc, 0xdb},
	{ 0x00, 0xc2, 0xd3,		0xdb, 0xc0},
	{ 0x00, 0xe3, 0x3e,		0xdd, 0xc0},
	{ 0x00, 0xed, 0x8d,		0xc0, 0xc0}



};

bool test_frame_fcsesc()
{
	int i;
	uint16_t fcs;
	int s, n;
	unsigned char enc[14];
	unsigned char dec[14];

	printf(" %s: ", __func__);

	for(i = 0; i < 16; i++)
	{
		fcs = INITFCS16;
		fcs = fcs16_byte(fcs, chkdata2[i][0]);
		fcs = fcs16_byte(fcs, chkdata2[i][1]);
		fcs = fcs16_byte(fcs, chkdata2[i][2]);
		fcs = fcs ^ 0xffff;

		s = encode_frame(chkdata2[i], enc, 3);
		n = decode_frame(enc, s, dec, sizeof(dec));
		
		printf("n = %d\n", n);


		if( (((fcs>>8)&0xff) != chkdata2[i][3]) ||  ((fcs&0xff) != chkdata2[i][4]))
		{
			dumpData(chkdata2[i], sizeof(chkdata2[i]), NULL);
			dumpData(enc, s, NULL);
			printf("FAIL! wrong FCS, got %04x, expected %02x%02x\n", fcs, chkdata2[i][3], chkdata2[i][4]);
			return false;
		}

		s = encode_frame(chkdata2[i], enc, 3);
		n = decode_frame(enc, s, dec, sizeof(dec));
		
		printf("n = %d\n", n);

		if( (dec[0] != chkdata2[i][0]) || (dec[1] != chkdata2[i][1]) || (dec[2] != chkdata2[i][2]) )
		{
			printf( "FAIL! Failed decode\n");
			dumpData(chkdata2[i], 3, NULL);
			dumpData(enc, s, NULL);
			return false;
		}
	}
	printf("PASS!\n");
	return true;
}

bool iskeyfcs(uint16_t fcs)
{
	static bool fl[16] = {};
	switch(fcs)
	{
		case 0xc0c0: if(!fl[0]) { fl[0] = true; return true; }
		case 0xc0db: if(!fl[1]) { fl[1] = true; return true; }
		case 0xc0dc: if(!fl[2]) { fl[2] = true; return true; }
		case 0xc0dd: if(!fl[3]) { fl[3] = true; return true; }

		case 0xdbc0: if(!fl[4]) { fl[4] = true; return true; }
		case 0xdbdb: if(!fl[5]) { fl[5] = true; return true; }
		case 0xdbdc: if(!fl[6]) { fl[6] = true; return true; }
		case 0xdbdd: if(!fl[7]) { fl[7] = true; return true; }

		case 0xdcc0: if(!fl[8]) { fl[8] = true; return true; }
		case 0xdcdb: if(!fl[9]) { fl[9] = true; return true; }
		case 0xdcdc: if(!fl[10]) { fl[10] = true; return true; }
		case 0xdcdd: if(!fl[11]) { fl[11] = true; return true; }

		case 0xddc0: if(!fl[12]) { fl[12] = true; return true; }
		case 0xdddb: if(!fl[13]) { fl[13] = true; return true; }
		case 0xdddc: if(!fl[14]) { fl[14] = true; return true; }
		case 0xdddd: if(!fl[15]) { fl[15] = true; return true; }
			return false;

		default:
			return false;
	}

}


bool test_frame_keyvalues()
{
	unsigned char data[4] = {0,0,0,0};
    uint16_t fcs;
	int i;
	int j;
	int k;
	int l;
	int s;
	int n;
	unsigned char enc[14];
	unsigned char dec[14];

	printf(" %s: ", __func__);

	for(i = 0; i < 256; i++)
	{
		fcs = INITFCS16;
		data[0] = i;
		fcs = fcs16_byte(fcs, data[0]);
		fcs = fcs ^ 0xffff;

		s = encode_frame(data, enc, 1);
		n = decode_frame(enc, s, dec, sizeof(dec));
		
		printf("n = %d\n", n);

		if( (1 != n) || (dec[0] != data[0]) )
		{
			printf("Failed to encode/decode: ");
			printf("%02x %04x\n", data[0], fcs);
			dumpData(enc, s,NULL);
			dumpData(dec, n,NULL);
			return false;
		}

		// Assert just incase
		assert(1 == n);
		assert(dec[0] == data[0]);
	}
	printf("Pass1... ");
	fflush(stdout);

	for(i = 0; i < 256; i++)
	{
		for(j = 0; j < 256; j++)
		{
			fcs = INITFCS16;
			data[0] = i;
			data[1] = j;
			fcs = fcs16_byte(fcs, data[0]);
			fcs = fcs16_byte(fcs, data[1]);
			fcs = fcs ^ 0xffff;

			s = encode_frame(data, enc, 2);
			n = decode_frame(enc, s, dec, sizeof(dec));

            printf("n = %d\n", n);

			if( (2 != n) || (dec[0] != data[0]) || (dec[1] != data[1]) )
			{
				printf("Failed to encode/decode: ");
				printf("%02x %02x %04x\n", data[0], data[1], fcs);
				dumpData(enc, s,NULL);
				dumpData(dec, n,NULL);
				return false;
			}

			assert(2 == n);
			assert(dec[0] == data[0]);
			assert(dec[1] == data[1]);
		}
	}

	// At this point the FCS values contained an escaped sequence in both octets.
	// TODO: Test for FCS values c0c0, dbdb, dcdc, dddd, c0db, etc...

	printf("Pass2...");
	fflush(stdout);

	for(i = 0; i < 256; i++)
	{
		for(j = 0; j < 256; j++)
		{
			for(k = 0; k < 256; k++)
			{
				fcs = INITFCS16;
				data[0] = i;
				data[1] = j;
				data[2] = k;
				fcs = fcs16_byte(fcs, data[0]);
				fcs = fcs16_byte(fcs, data[1]);
				fcs = fcs16_byte(fcs, data[2]);
				fcs = fcs ^ 0xffff;

				s = encode_frame(data, enc, 3);
				n = decode_frame(enc, s, dec, sizeof(dec));
				
				printf("n = %d\n", n);

#if 0
				// Make table for testing
				if(iskeyfcs(fcs))
				{
					//printf("\nKeyFCS\n");
					printf("{ 0x%02x, 0x%02x, 0x%02x,\t\t0x%02x, 0x%02x},\n", data[0], data[1], data[2], (fcs>>8)&0xff, fcs&0xff);
					//printf("\n");
				}
#endif

				if( (3 != n) || (dec[0] != data[0]) || (dec[1] != data[1]) || (dec[2] != data[2]) )
				{
					printf("Failed to encode/decode: ");
					printf("%02x %02x %02x %04x\n", data[0], data[1], data[2], fcs);
					dumpData(enc, s,NULL);
					dumpData(dec, n,NULL);
					return false;
				}

				assert(3 == n);
				assert(dec[0] == data[0]);
				assert(dec[1] == data[1]);
				assert(dec[2] == data[2]);
			}
		}
	}

	printf("Pass3...");
	fflush(stdout);

	int nib = -1;
	int maxlen = 0;

	for(i = 0; i < 256; i++)
	{
		if( (i >> 4) != nib )
		{
			nib = i >> 4;
			printf("%c", nib + '0');
			fflush(stdout);
		}
		for(j = 0; j < 256; j++)
		{
			for(k = 0; k < 256; k++)
			{
				for(l = 0; l < 256; l++)
				{
					fcs = INITFCS16;
					data[0] = i;
					data[1] = j;
					data[2] = k;
					data[3] = l;
					fcs = fcs16_byte(fcs, data[0]);
					fcs = fcs16_byte(fcs, data[1]);
					fcs = fcs16_byte(fcs, data[2]);
					fcs = fcs16_byte(fcs, data[3]);
					fcs = fcs ^ 0xffff;

					s = encode_frame(data, enc, 4);
					n = decode_frame(enc, s, dec, sizeof(dec));

					if(s > maxlen)
					{
						maxlen = s;
						dumpData(data,4, NULL);
						dumpData(enc,s, NULL);
					}


					if( (4 != n) || (dec[0] != data[0]) || (dec[1] != data[1]) || (dec[2] != data[2]) || (dec[3] != data[3]))
					{
						printf("Failed to encode/decode: ");
						printf("%02x %02x %02x %02x %04x\n", data[0], data[1], data[2], data[3], fcs);
						dumpData(enc, s,NULL);
						dumpData(dec, n,NULL);
						return false;
					}

					assert(4 == n);
					assert(dec[0] == data[0]);
					assert(dec[1] == data[1]);
					assert(dec[2] == data[2]);
					assert(dec[3] == data[3]);
				}
			}
		}
	}

	printf("Pass4 ");
	fflush(stdout);

	printf("PASS\n");

#if 0
	fcs = INITFCS16;
	fcs = fcs16_byte(fcs, 0xff);
	fcs = fcs16_byte(fcs, 0xff);
	fcs = fcs16_byte(fcs, 0xff);
	fcs = fcs16_byte(fcs, 0xff);
	fcs = fcs16_byte(fcs, 0xff);
	//fcs = fcs16_byte(fcs, 0x00);
	//fcs = fcs16_byte(fcs, 0x00);
	fcs = fcs ^ 0xffff;
	printf("FCS: %04x\n", fcs);

#endif


	return true;



}

bool test_fcs16()
{
	printf("Testing FCS16:\n");
	assert(test_fcs16_sanity());
	assert(test_fcs16_values(__fcs16_test2, sizeof(__fcs16_test2), 0xc3e9, 0x16, 0x3c));
	assert(test_fcs16_stream());
	assert(test_fcs16_allvalues());

	printf("FCS16: Passed\n");
	return true;
}




bool test_frame()
{
	frame_t frame;
	unsigned char buffer[8*1024];
	//unsigned char buf[] = { 1, 2, 3, 0xc0, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0x16, 0x3c , 0xc0};
	//unsigned char buf[] = { 1, 2, 3, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0x16, 0x3c , 0xc0};
	unsigned char buf[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0x16, 0x3c , 0xc0};
	//unsigned char buf[] = { 0, '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0x16, 0x3c , 0xc0};
	int i;

	frame_setbuffer(&frame, buffer, sizeof(buffer));

	for(i = 0; i < sizeof(buf); i++)
	{
		//printf("Write %02x\n", buf[i]);
		if(frame_input(&frame, buf[i]/*, dumpData, NULL*/))
		{
			printf("Got a frame:\n");
			dumpData(frame.data, frame.data_len, NULL);
			printf("\n");
			frame_reset(&frame);
		}
	}


	return true;
}


bool test_encode()
{
	//frame_t frame;
	//unsigned char buffer[8*1024];

	/*
	const uint8_t testdata1[] = "HelloWorld\n";
	const uint8_t validate1[] = { 0xc0, 'H', 'e', 'l', 'l', 'o', 'W', 'o', 'r', 'l', 'd', '\n', '\0', 0xbc, 0x97, 0xc0 };
	*/

	//unsigned char buf[] = "Hello World\xc0";
	//unsigned char buf[] = { 0xff, 0xff };
	// Makes and esc appear in FCS16
	unsigned char buf[] = { 0x00, 0x0b, 0x07 };
	//unsigned char buf[] = "Hello World\n";
	unsigned char buffer2[16*1024];
	unsigned char bufout[16*1024];
	size_t len;

	int s ;

	printf("Data:\n");
	dumpData(buf, sizeof(buf), NULL);
	printf("\n");

	s = encode_frame(buf, buffer2, sizeof(buf));

	printf("Encoded Frame:\n");
	dumpData(buffer2, s, NULL);
	printf("\n");

	len = decode_frame(buffer2, s, bufout, sizeof(bufout));
	printf("Decoded Frame:\n");
	dumpData(bufout, len, NULL);
	printf("\n");

	return true;


}


#include <unistd.h>
#include <signal.h>

int main(int argc, char *argv[])
{

	test_fcs16();

	test_frame();

	test_encode();

	assert(test_frame_fcsesc());
	//assert(test_frame_keyvalues());

	printf("\n");


	void serverLoop();
	void clientLoop();

	if(argc > 1 && argv[1][0] == 'c')
		clientLoop();
	if(argc > 1 && argv[1][0] == 's')
		serverLoop();
	/*
	pid_t serverpid;
	pid_t clientpid;

	serverpid = fork();
	if(0 == serverpid)
	{
		serverLoop();
		return 0;
	}
	if(serverpid < 0)
	{
		perror("fork");
		exit(-1);
	}





	printf("Server proc %i\n", serverpid);


	clientpid = fork();
	if(0 == clientpid)
	{
		clientLoop();
		return 0;
	}
	if(clientpid < 0)
	{
		perror("fork");
		exit(-1);
	}

	printf("client proc %i\n", clientpid);


	int st;
	pid_t pid;
	pid = wait(&st);
	if(pid == serverpid)
	{
		printf("server exitd with %i\n", st);
		kill(clientpid, SIGQUIT);
		return 0;
	}
	else if( pid == clientpid)
	{
		printf("client exitd with %i\n", st);
		kill(serverpid, SIGQUIT);
		return 0;
	}
	*/



	return 0;
}
