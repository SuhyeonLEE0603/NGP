#include "Common.h"

bool HostByte(u_long n) {
	u_long result = htonl(n);

	printf("호스트 바이트 -> %#x\n", n);
	printf("네트워크 바이트 -> %#x\n", result);

	if (n == result) {
		printf("호스트 바이트 정렬 방식은 빅앤디언입니다.\n");
		return true;
	}
	else {
		printf("호스트 바이트 정렬 방식은 리틀앤디언입니다.\n");
		return false;
	}
}

int main(int argc, char* argv[])
{
	u_long x1 = 0x12345678;

	HostByte(x1);

	u_long x2 = 0x78123478;

	HostByte(x2);
	
	u_long x3 = 0x11111111;

	HostByte(x3);

	return 0;
}
