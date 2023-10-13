#include "../Common.h"

#define SERVERPORT 9000
#define BUFSIZE    1024

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
	///////////////////////////////////////////////////
	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE];			// ���� ���� ������
	char fileName[256];			// ���� �̸�
	memset(fileName, 0, sizeof(fileName));
	int filesize;				// ���� ũ��
	int size{ 256 };
	///////////////////////////////////////////////////

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


		// Ŭ���̾�Ʈ�� ������ ���
		// ������ �ޱ�(���ϸ�)
		retval = recv(client_sock, fileName, sizeof(fileName), 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv() ���ϸ�");
			break;
		}

		fileName[retval] = 0;
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
			break;
		}

		// ������ �ޱ�(���� ������)
		int numtotal = 0;
		while ((retval = recv(client_sock, buf, BUFSIZE, 0)) > 0) {
			// ������ �ޱ�(���� ������)
			// 1 KB�� �ޱ�
			if (retval == SOCKET_ERROR) {
				err_display("recv() ���� ������");
				break;
			}
			fwrite(buf, sizeof(char), retval, fp);
			if (ferror(fp)) {
				printf("���� ����� ����2");
				break;
			}

			numtotal += retval;
			printf("���۷�(���ŷ�) : %0.2f %%\n", (float)numtotal / filesize * 100);

		}
		fclose(fp);

		printf("[TCP ����] Ŭ���̾�Ʈ ���ۼ��� : IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// ���� �ݱ�
		closesocket(client_sock);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}