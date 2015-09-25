#include <iostream>



class Serial
{
	public:
		virtual ~Serial();

		virtual void write(const uint8_t * data, size_t len) = 0;
		virtual ssize_t read(uint8_t * data, size_t len) = 0;
};


class TestSerial : public Serial
{
	public:
		TestSerial();
		virtual ~TestSerial();

		virtual void write(const uint8_t * data, size_t len);
		virtual ssize_t read(uint8_t * data, size_t len);


};

Serial::~Serial() {}

TestSerial::TestSerial() { }
TestSerial::~TestSerial() { }

void TestSerial::write(const uint8_t * data, size_t len)
{
	int i;
	for(i = 0; i < len; i++)
		printf("%02x", data[i]);
	printf("\n");
}

ssize_t TestSerial::read(uint8_t * data, size_t len)
{
	return 0;
}




int main(int argc, char * argv[])
{

	TestSerial ts;

	const uint8_t * hello = (const uint8_t*)"Hello World\n";

	ts.write(hello, strlen((const char *)hello));




	return 0;
}
