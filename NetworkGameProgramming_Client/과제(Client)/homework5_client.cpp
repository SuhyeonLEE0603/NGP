#include "../../Common.h"

#define SERVERPORT 9000
#define BUFSIZE    20000

int main(int argc, char* argv[])
{
	// ������ ��ſ� ����� ����
	///////////////////////////////////////////////////
	int retval;
	char SERVERIP[INET6_ADDRSTRLEN];	// ���� IP�ּ�
	char buf[BUFSIZE];
	int fileNameLen;					// ���ϸ� ����
	char fileName[256];					// ���ϸ�
	int filesize;						// ���� ũ��
	int numread;						// ���� ����Ʈ ��
	int numtotal = 0;					// �� ���� ������ ��
	///////////////////////////////////////////////////


	// ����� �μ��� ���� �̸����� ���
	if (argc > 1) {
		strcpy(SERVERIP, argv[1]);
		strcpy(fileName, argv[2]);
	}
	else {
		printf("���� �̸��� �Է��ϼ���");
		return 1;
	}

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
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


	// ���� ����
	FILE* fp = fopen(fileName, "rb");
	if (fp == NULL) {
		printf("���� ����� ����1");
		return 1;
	}
	// ������ ������ ���

	// ������ ������(���ϸ� ����)
	fileNameLen = strlen(fileName);
	retval = send(sock, (char*)&fileNameLen, sizeof(fileNameLen), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() ���ϸ� ����");
		return 1;
	}

	// ������ ������(���ϸ�)
	retval = send(sock, fileName, fileNameLen, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send() ���ϸ�");
		return 1;
	}

	printf("���� ���� �̸� -> %s\n", fileName);

	// ���� ũ�� ������
	fseek(fp, 0, SEEK_END);
	filesize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	retval = send(sock, (char*)&filesize, sizeof(filesize), 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return 1;
	}

	 //������ ������(���� ������)
	 //1KB�� �б�

	while (numread = fread(buf, sizeof(char), BUFSIZE, fp)) {
		if (numread > 0) {
			// ������ ������(���� ������)
			retval = send(sock, buf, numread, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send() ���� ������");
				return 1;
			}

			numtotal += retval;
		}
	}
	printf("[TCP Ŭ���̾�Ʈ] ���� ���� �Ϸ� %d����Ʈ�� ���½��ϴ�.\n",
		numtotal);

	fclose(fp);

	// ���� �ݱ�
	closesocket(sock);

	// ���� ����
	WSACleanup();
	return 0;
}

// cd C:\NetworkGameProgramming_Client\x64\Release
// NetworkGameProgramming_Client 127.0.0.1 testvideo.mp4