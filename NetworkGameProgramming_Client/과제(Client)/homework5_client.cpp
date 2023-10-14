#include "../../Common.h"

#define SERVERPORT 9000
#define BUFSIZE    20000

int main(int argc, char* argv[])
{
	// 데이터 통신에 사용할 변수
	///////////////////////////////////////////////////
	int retval;
	char SERVERIP[INET6_ADDRSTRLEN];	// 서버 IP주소
	char buf[BUFSIZE];
	int fileNameLen;					// 파일명 길이
	char fileName[256];					// 파일명
	int filesize;						// 파일 크기
	int numread;						// 읽은 바이트 수
	int numtotal = 0;					// 총 보낸 데이터 수
	///////////////////////////////////////////////////


	// 명령행 인수를 파일 이름으로 사용
	if (argc > 1) {
		strcpy(SERVERIP, argv[1]);
		strcpy(fileName, argv[2]);
	}
	else {
		printf("파일 이름을 입력하세요");
		return 1;
	}

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");

	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("connect()");


	// 파일 열기
	FILE* fp = fopen(fileName, "rb");
	if (fp == NULL) {
		printf("파일 입출력 오류1");
		return 1;
	}
	// 서버와 데이터 통신

	// 데이터 보내기(파일명 길이)
	fileNameLen = strlen(fileName);
	retval = send(sock, (char*)&fileNameLen, sizeof(fileNameLen), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() 파일명 길이");
		return 1;
	}

	// 데이터 보내기(파일명)
	retval = send(sock, fileName, fileNameLen, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() 파일명");
		return 1;
	}

	printf("보낼 파일 이름 -> %s\n", fileName);

	// 파일 크기 보내기
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	retval = send(sock, (char*)&filesize, sizeof(filesize), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 1;
	}

	 //데이터 보내기(파일 데이터)
	 //1KB씩 읽기

	while (numread = fread(buf, sizeof(char), BUFSIZE, fp)) {
		if (numread > 0) {
			// 데이터 보내기(파일 데이터)
			retval = send(sock, buf, numread, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send() 파일 데이터");
				return 1;
			}

			numtotal += retval;
		}
	}
	printf("[TCP 클라이언트] 파일 전송 완료 %d바이트를 보냈습니다.\n",
		numtotal);

	fclose(fp);

	// 소켓 닫기
	closesocket(sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

// cd C:\NetworkGameProgramming_Client\x64\Release
// NetworkGameProgramming_Client 127.0.0.1 testvideo.mp4