#include <stdlib.h>
#include <stdio.h>

#include <stdint.h>
#include <stdbool.h>
#include "frame.h"
#include "fcs16.h"

#include <ctype.h>

#define _I(f) #f

const uint8_t testdata1[] = "HelloWorld\n";
const uint8_t validate1[] = { 0xc0, 'H', 'e', 'l', 'l', 'o', 'W', 'o', 'r', 'l', 'd', '\n', '\0', 0xbc, 0x97, 0xc0 };

const uint8_t testdata2[] = "Hello\xc0World\n";
const uint8_t validate2[] = { 0xc0, 'H', 'e', 'l', 'l', 'o', 0xdb, 0xdc, 'W', 'o', 'r', 'l', 'd', '\n', '\0', 0x53, 0x24, 0xc0 };

const uint8_t testdata3[] = "Hello\xdbWorld\n";
const uint8_t validate3[] = { 0xc0, 'H', 'e', 'l', 'l', 'o', 0xdb, 0xdd, 'W', 'o', 'r', 'l', 'd', '\n', '\0', 0x47, 0xd8, 0xc0 };

const uint8_t testdata4[] = "Hello\xddWorld\n";
const uint8_t validate4[] = { 0xc0, 'H', 'e', 'l', 'l', 'o', 0xdd, 'W', 'o', 'r', 'l', 'd', '\n', '\0', 0xf6, 0xc5, 0xc0 };

const uint8_t testdata5[] = "Hello\xdcWorld\n";
const uint8_t validate5[] = { 0xc0, 'H', 'e', 'l', 'l', 'o', 0xdc, 'W', 'o', 'r', 'l', 'd', '\n', '\0', 0x49, 0x44, 0xc0 };

// Note the FCS16 needs to be escaped,
const uint8_t testdata6[] = { 0x00, 0x0b, 0x07 };
const uint8_t validate6[] = { 0xc0, 0x00, 0x0b, 0x07, 0xdb, 0xdd, 0x56, 0xc0 };


bool encodeTestData(const uint8_t * testdata, size_t testlen, const uint8_t * validate, size_t validlen)
{
	uint8_t buffer[8192];
	size_t len = 0;
	int i;

	len = encodeFrame(testdata, testlen, buffer);
	if(len != validlen)
	{
		printf("FAIL! encoded len is %u, expected %u\n", (unsigned int)len, (unsigned int)validlen);
		return false;
	}
	for(i = 0; i < len; i++)
	{
		if(validate[i] != buffer[i])
		{
			printf("FAIL! buffer[%i] == %02x, expecting %02x\n", i, buffer[i], validate[i]);
			return false;
		}
	}

	printf("PASS\n");
	return true;
}

#define TESTENC(a,b) { printf("Testing " _I(a) ": "); if(!encodeTestData(a,sizeof(a),b,sizeof(b))) return false; }

bool encodeTest()
{
	TESTENC(testdata1,validate1);
	TESTENC(testdata2,validate2);
	TESTENC(testdata3,validate3);
	TESTENC(testdata4,validate4);
	TESTENC(testdata5,validate5);
	TESTENC(testdata6,validate6);

	return true;
}

bool decodeTestData(const uint8_t * testdata, size_t testlen, const uint8_t * validData, size_t validlen)
{
	int i;
	Frame_t frame = {};
	resetFrame(&frame);

	for(i = 0; i < testlen; i++)
	{
		if(putByte(&frame, testdata[i]))
		{
			if(frame.length == 0)
				continue; // ignore empty frame;
			if(validlen + 2 != frame.length)
			{
				printf("FAIL! frame length incorrect, got %u expecting %u\n", (unsigned int)frame.length, (unsigned int)validlen + 2);
				return false;
			}
			if(frame.fcs != GOODFCS16)
			{
				printf("fcs = %04x, expecting %04x\n", frame.fcs, GOODFCS16 );
				return false;
			}
			{
				int j;
				for(j = 0; j < frame.length - 2; j++)
				{
					if(validData[j] != frame.buffer[j])
					{
						printf("FAIL! frame.biffer[%i] == %02x, expecting %02x\n", j, frame.buffer[j], validData[j]);
						return false;
					}
				}


			}
			printf("PASS!\n");
			return true;
		}
	}

	return false;
}


bool decodeTest()
{

	decodeTestData(validate1, sizeof(validate1), testdata1, sizeof(testdata1));


	return true;
}

int main(int argc, char * argv[])
{
	
	printf("Encoding Test:\n");
	if(!encodeTest())
		return EXIT_FAILURE;

	decodeTest();

	return 0;
}
