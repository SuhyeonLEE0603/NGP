#include "Common.h"

bool HostByte(u_long n) {
	u_long result = htonl(n);

	printf("ȣ��Ʈ ����Ʈ -> %#x\n", n);
	printf("��Ʈ��ũ ����Ʈ -> %#x\n", result);

	if (n == result) {
		printf("ȣ��Ʈ ����Ʈ ���� ����� ��ص���Դϴ�.\n");
		return true;
	}
	else {
		printf("ȣ��Ʈ ����Ʈ ���� ����� ��Ʋ�ص���Դϴ�.\n");
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
