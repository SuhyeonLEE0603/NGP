#include "../Common.h"

#define SERVERPORT 9000
#define BUFSIZE    20000

// Ŭ���̾�Ʈ�� ������ ���
DWORD WINAPI ProcessClient(LPVOID arg)
{
	// ������ ��ſ� ����� ����
	///////////////////////////////////////////////////
	int retval;
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char buf[BUFSIZE];										// ���� ������ ���� ����
	int fileNameLen;										// ���ϸ� ����
	char fileName[256];										// ���ϸ�
	int filesize{};											// ���� ũ��
	int numtotal{};											// ������ ������ ����Ʈ ��
	int reception_rate{};									// ���ŷ�
	///////////////////////////////////////////////////

	// Ŭ���̾�Ʈ ���� ���
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	// Ŭ���̾�Ʈ�� ������ ���

	// ������ �ޱ�(���ϸ� ����)
	retval = recv(client_sock, (char*)&fileNameLen, sizeof(fileNameLen), 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv() ���ϸ� ����");
		return 1;
	}

	// ������ �ޱ�(���ϸ�)
	retval = recv(client_sock, fileName, fileNameLen, 0);
	if (retval == SOCKET_ERROR) {
		err_display("recv() ���ϸ�");
		return 1;
	}
		
	printf("���� ���� �̸� -> %s\n", fileName);

	// ���� ũ�� �ޱ�
	retval = recv(client_sock, (char*)&filesize, sizeof(filesize), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 1;
	}

	// ����� ���� ����(���� ���� �̸�)
	FILE* fp = fopen(fileName, "wb");
	if (fp == NULL) {
		printf("���� ����� ����1");
		return 1;
	}

	// ������ �ޱ�(���� ������)
	while ((retval = recv(client_sock, buf, BUFSIZE, 0)) > 0) {
		// ������ �ޱ�(���� ������)
		// 1 KB�� �ޱ�
		if (retval == SOCKET_ERROR) {
			err_display("recv() ���� ������");
			return 1;
		}
		fwrite(buf, sizeof(char), retval, fp);
		if (ferror(fp)) {
			printf("���� ����� ����2");
			return 1;
		}

		numtotal += retval;
		reception_rate = (int)((float)numtotal / filesize * 100);
		if (reception_rate % 5 == 0) {
			printf("���۷�(���ŷ�) : %d %%\n", reception_rate);
		}
	}
	fclose(fp);

	printf("[TCP ����] Ŭ���̾�Ʈ ���ۼ��� : IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));
	
	// ���� �ݱ�
	closesocket(client_sock);
	printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
		addr, ntohs(clientaddr.sin_port));
}

int main(int argc, char* argv[])
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
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

	// ������ ��ſ� ����� ����
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

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
		
		// ������ ����
		hThread = CreateThread(NULL, 0, ProcessClient,
			(LPVOID)client_sock, 0, NULL);
		SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
		if (hThread == NULL) { 
			closesocket(client_sock); 
		}
		else { 
			CloseHandle(hThread); 
		}
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}