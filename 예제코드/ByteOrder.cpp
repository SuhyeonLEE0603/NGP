#include "Common.h"

bool HostByte(u_short n) {
	u_short result = htons(n);

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

int main(int argc, char *argv[])
{
	u_short x1 = 0x1234;

	HostByte(x1);

	return 0;
}
