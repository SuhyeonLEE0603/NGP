#include "Common.h"

bool HostByte(u_short n) {
	u_short result = htons(n);

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

int main(int argc, char *argv[])
{
	u_short x1 = 0x1234;

	HostByte(x1);

	return 0;
}
