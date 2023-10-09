#include "Common.h"

int main(int argc, char *argv[])
{
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;
	printf("[�˸�] ���� �ʱ�ȭ ����\n");
	
	// ���� 8��Ʈ, ���� 8��Ʈ ����
	BYTE Versionlowbyte = LOBYTE(wsa.wVersion);
	BYTE Versionhighbyte = HIBYTE(wsa.wVersion);
	BYTE HighVersionlowbyte = LOBYTE(wsa.wHighVersion);
	BYTE HighVersionhighbyte = HIBYTE(wsa.wHighVersion);

	// ����01
	printf("wVersion: %d.%d, wHighVersion: %d.%d, szDescription: %s, szSystemStatus: %s\n",
		Versionlowbyte, Versionhighbyte, HighVersionlowbyte, HighVersionhighbyte, wsa.szDescription, wsa.szSystemStatus);
	
	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[�˸�] ���� ���� ����\n");

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;

}
