#include "Common.h"

void printall(char* name) {
    int namelen = sizeof(name);

    hostent* host = gethostbyname(name);

    printf("공식 도메인 이름 : %s\n", host->h_name);

    printf("별명 : ");
    for (int i = 0; host->h_aliases[i] != NULL; ++i) {
        printf("%s\n", host->h_aliases[i]);
    }

    printf("IP주소 : ");
    char str[INET_ADDRSTRLEN];

    struct in_addr addr;

    for (int i = 0; host->h_addr_list[i] != NULL; ++i) {
        memcpy(&addr, host->h_addr_list[i], host->h_length);
        printf("%s\n", inet_ntop(AF_INET, &addr, str, sizeof(str)));
    }

}

int main(int argc, char* argv[]) {

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;
    for (int i = 1; i < argc; ++i) {
        printall(argv[i]);
    }
 
    // 윈속 종료
    WSACleanup();
    return 0;
}
