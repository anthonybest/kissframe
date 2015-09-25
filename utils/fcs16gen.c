#include <stdio.h>

/*
* Generate a FCS-16 table.
*
* Drew D. Perkins at Carnegie Mellon University.
*
* Code liberally borrowed from Mohsen Banan and D. Hugh Redelmeier.
*/

/*
* The FCS-16 generator polynomial: x**0 + x**5 + x**12 + x**16.
*/

#define P       0x8408

int main(int argc, char * argv[])
{
	unsigned int b, v;
	int i;

	printf("static uint16_t fcstab[256] = {");
	for (b = 0;;)
	{
		if (b % 8 == 0)
			printf("\n");

		v = b;
		for (i = 8; i--;)

			v = v & 1 ? (v >> 1) ^ P : v >> 1;

		printf("\t0x%04x", v & 0xFFFF);
		if (++b == 256)
			break;
		printf(",");
	}
	printf("\n};\n");
}
