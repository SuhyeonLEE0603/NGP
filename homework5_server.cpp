#include "Common.h"

#define SERVERPORT 9000
#define BUFSIZE    1024

int main(int argc, char* argv[])
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수
	///////////////////////////////////////////////////
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE];			// 가변 길이 데이터
	char fileName[256];			// 파일 이름
	memset(fileName, 0, sizeof(fileName));
	int filesize;				// 파일 크기
	int size{256};
	///////////////////////////////////////////////////

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));


		// 클라이언트와 데이터 통신
		// 데이터 받기(파일명)
		retval = recv(client_sock, fileName, 13, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv() 파일명");
			break;
		}

		printf("받을 파일 이름 -> %s\n", fileName);

		// 파일 크기 받기
		retval = recv(client_sock, (char*)&filesize, sizeof(filesize), 0);
		if (retval == SOCKET_ERROR) {
			err_display("send()");
			return 1;
		}

		// 쓰기용 파일 열기(같은 파일 이름)
		FILE* fp = fopen(fileName, "wb");
		if (fp == NULL) {
			printf("파일 입출력 오류1");
			break;
		}

		// 데이터 받기(파일 데이터)
		int numtotal = 0;
		while ((retval = recv(client_sock, buf, BUFSIZE, 0)) > 0) {
			// 데이터 받기(파일 데이터)
			// 1 KB씩 받기
			if (retval == SOCKET_ERROR) {
				err_display("recv() 파일 데이터");
				break;
				}
			fwrite(buf, sizeof(char), retval, fp);
			if (ferror(fp)) {
				printf("파일 입출력 오류2");
				break;
				}
					
			numtotal += retval;
			printf("전송률(수신률) : %0.2f %%\n", (float)numtotal / filesize * 100);

			}
		fclose(fp);
			
		printf("[TCP 서버] 클라이언트 전송성공 : IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 소켓 닫기
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}
