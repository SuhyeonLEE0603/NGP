#include "../../Common.h"

#define SERVERPORT 9000
#define BUFSIZE    1024

int ThreadNum{};

// 표준 출력 핸들을 얻음
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
// 현재 커서위치를 저장하기 위한 변수
COORD cursorPos;

// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	++ThreadNum;
	int num = ThreadNum;

	// 커서를 (X, Y) 좌표로 이동
	cursorPos.X = 0; // X 좌표
	cursorPos.Y = 6 * (num - 1); // Y 좌표

	SetConsoleCursorPosition(hConsole, cursorPos);

	// 데이터 통신에 사용할 변수
	///////////////////////////////////////////////////
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE];										// 파일 데이터 수신 버퍼
	int fileNameLen;										// 파일명 길이
	char fileName[256];										// 파일명
	int filesize{};											// 파일 크기
	int numtotal{};											// 수신한 데이터 바이트 수
	///////////////////////////////////////////////////

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	printf("[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));

	// 클라이언트와 데이터 통신
	// 데이터 받기(파일명 길이)
	retval = recv(client_sock, (char*)&fileNameLen, sizeof(fileNameLen), 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv() 파일명 길이");
		return 1;
	}

	// 데이터 받기(파일명)
	retval = recv(client_sock, fileName, fileNameLen, 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv() 파일명");
		return 1;
	}
	cursorPos.Y = 6 * (num - 1) + 1; // Y 좌표

	SetConsoleCursorPosition(hConsole, cursorPos);
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
		return 1;
	}

	// 데이터 받기(파일 데이터)
	while ((retval = recv(client_sock, buf, BUFSIZE, 0)) > 0) {
		// 데이터 받기(파일 데이터)
		// 1 KB씩 받기
		if (retval == SOCKET_ERROR) {
			err_display("recv() 파일 데이터");
			return 1;
		}
		fwrite(buf, sizeof(char), retval, fp);
		if (ferror(fp)) {
			printf("파일 입출력 오류2");
			return 1;
		}

		numtotal += retval;
		cursorPos.Y = 6 * (num - 1) + 2; // Y 좌표

		SetConsoleCursorPosition(hConsole, cursorPos);
		printf("클라이언트(IP 주소=%s, 포트 번호=%d) 전송률(수신률) = %0.2f %% \r",
			addr, ntohs(clientaddr.sin_port), (float)numtotal / filesize * 100);

	}

	fclose(fp);
	cursorPos.Y = 6 * (num - 1) + 2; // Y 좌표

	SetConsoleCursorPosition(hConsole, cursorPos);
	if (((float)numtotal / filesize * 100) == 100.0) {
		printf("\n[TCP 서버] 클라이언트 전송성공 : IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}
	else {
		printf("[TCP 서버] 클라이언트 전송실패 : IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}
	
	// 소켓 닫기
	closesocket(client_sock);

	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));

}

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
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	HANDLE hThread;

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 스레드 생성
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);
		
		if (hThread == NULL) { 
			closesocket(client_sock); 
		}
		else { 
			CloseHandle(hThread); 
		}
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}