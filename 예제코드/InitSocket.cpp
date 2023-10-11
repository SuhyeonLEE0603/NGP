#include "Common.h"

int main(int argc, char *argv[])
{
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(0x0202, &wsa) != 0)
		return 1;
	printf("[알림] 윈속 초기화 성공\n");
	
	// 하위 8비트, 상위 8비트 구분
	BYTE Versionlowbyte = LOBYTE(wsa.wVersion);
	BYTE Versionhighbyte = HIBYTE(wsa.wVersion);
	BYTE HighVersionlowbyte = LOBYTE(wsa.wHighVersion);
	BYTE HighVersionhighbyte = HIBYTE(wsa.wHighVersion);

	// 과제01
	printf("wVersion: %d.%d, wHighVersion: %d.%d, szDescription: %s, szSystemStatus: %s\n",
		Versionlowbyte, Versionhighbyte, HighVersionlowbyte, HighVersionhighbyte, wsa.szDescription, wsa.szSystemStatus);
	
	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	printf("[알림] 소켓 생성 성공\n");

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;

}
