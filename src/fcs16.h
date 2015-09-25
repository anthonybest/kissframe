#include <stdint.h>

#define INITFCS16    0xffff	
#define GOODFCS16    0xf0b8	

uint16_t fcs16_byte(uint16_t fcs, unsigned char c);
unsigned char * fcs16_compliment(uint16_t fcs, unsigned char * cp);
uint16_t fcs16(uint16_t fcs, unsigned char * data, size_t len);
