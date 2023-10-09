#include "Common.h"

void printall(char* name) {
    int namelen = sizeof(name);

    hostent* host = gethostbyname(name);

    printf("���� ������ �̸� : %s\n", host->h_name);

    printf("���� : ");
    for (int i = 0; host->h_aliases[i] != NULL; ++i) {
        printf("%s\n", host->h_aliases[i]);
    }

    printf("IP�ּ� : ");
    char str[INET_ADDRSTRLEN];

    struct in_addr addr;

    for (int i = 0; host->h_addr_list[i] != NULL; ++i) {
        memcpy(&addr, host->h_addr_list[i], host->h_length);
        printf("%s\n", inet_ntop(AF_INET, &addr, str, sizeof(str)));
    }

}

int main(int argc, char* argv[]) {

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;
    for (int i = 1; i < argc; ++i) {
        printall(argv[i]);
    }
 
    // ���� ����
    WSACleanup();
    return 0;
}
