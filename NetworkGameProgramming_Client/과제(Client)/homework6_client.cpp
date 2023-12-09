#include "../../Common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    4096

HWND hEdit, hSendButton, hProgressBar;
char fileName[256];

// 함수 프로토 타입
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void OpenFileSelectionDialog(HWND hWnd);
void SendFiles(HWND hWnd, SOCKET sock);

// 선택한 파일 개수
int file_cnt{ 0 };

SOCKET sock;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // 윈도우 클래스 등록
    const wchar_t CLASS_NAME[] = L"homework6";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    int retval;

    // 윈속 초기화
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // 소켓 생성
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) err_quit("socket()");

    // connect()
    struct sockaddr_in serveraddr;
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(SERVERPORT);
    retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if (retval == SOCKET_ERROR) err_quit("connect()");


    // 윈도우 생성
    HWND hWnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"homework6",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL, NULL, hInstance, NULL
    );

    if (hWnd == NULL) {
        return 0;
    }

    // 에디트 컨트롤 생성
    hEdit = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        L"EDIT",
        NULL,
        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        10, 10, 400, 200,
        hWnd,
        NULL,
        hInstance,
        NULL
    );

    // 파일 선택 버튼 생성
    CreateWindowEx(
        0,
        L"BUTTON",
        L"파일선택",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 220, 120, 30,
        hWnd,
        (HMENU)1,
        hInstance,
        NULL
    );

    // Send 버튼 생성
    hSendButton = CreateWindowEx(
        0,
        L"BUTTON",
        L"보내기",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        140, 220, 80, 30,
        hWnd,
        (HMENU)2,
        hInstance,
        NULL
    );

    // 프로그래스 바 초기화

    hProgressBar = CreateWindowEx(
        0,
        PROGRESS_CLASS,
        NULL,
        WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
        10, 260, 400, 20,
        hWnd,
        NULL,
        hInstance,
        NULL
    );

    // 윈도우 표시
    ShowWindow(hWnd, nCmdShow);

    // 메시지 루프
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // 윈속 종료
	WSACleanup();
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_COMMAND:
        if (LOWORD(wParam) == 1) {
            OpenFileSelectionDialog(hWnd);
        }
        else if (LOWORD(wParam) == 2) {
            SendFiles(hWnd, sock);
        }
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}


void OpenFileSelectionDialog(HWND hWnd) {
    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    
    int len = MultiByteToWideChar(CP_ACP, 0, fileName, sizeof(fileName), NULL, 0);
    LPWSTR wideString = new WCHAR[len];

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = wideString;
    ofn.nMaxFile = len;
    ofn.lpstrFilter = L"All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST;

    if (GetOpenFileName(&ofn)) {

        // 파일명 추출
        TCHAR file_Name[256];
        ZeroMemory(&file_Name, sizeof(file_Name));
        lstrcpy(file_Name, PathFindFileName(ofn.lpstrFile));
        
        // 파일명 출력
        SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)L"파일 : ");
        SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)file_Name);
        SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)L"\r\n");

        // wideString을 char로 변환
        int mbLen = WideCharToMultiByte(CP_ACP, 0, file_Name, -1, NULL, 0, NULL, NULL);
        char* mbString = new char[mbLen];
        WideCharToMultiByte(CP_ACP, 0, file_Name, -1, mbString, mbLen, NULL, NULL);

        // fileName에 복사
        lstrcpyA(fileName, mbString);
        ++file_cnt;
        delete[] mbString;
    }
    delete[] wideString;
}


void SendFiles(HWND hWnd, SOCKET sock) {


    if (file_cnt == 0) {
        MessageBox(hWnd, L"파일이 선택되지 않음", L"전송 실패", MB_OK | MB_ICONINFORMATION);
    }
    else {
        // 데이터 통신에 사용할 변수
        ///////////////////////////////////////////////////
        int retval;
        char buf[BUFSIZE];
        int fileNameLen;					// 파일명 길이
        int filesize;						// 파일 크기
        int numread;						// 읽은 바이트 수
        int numtotal = 0;					// 총 보낸 데이터 수
        ///////////////////////////////////////////////////

        // 데이터 보내기(파일명 길이)
        fileNameLen = sizeof(fileName);
        TCHAR buffer[256];
        wsprintf(buffer, _T("전송된 파일명의 길이: %u"), fileNameLen);
        MessageBox(hWnd, buffer, L"전송 성공\n", MB_OK | MB_ICONINFORMATION);

        retval = send(sock, (char*)&fileNameLen, sizeof(fileNameLen), 0);

        // 데이터 보내기(파일명)
        retval = send(sock, (char*)fileName, fileNameLen * sizeof(TCHAR), 0);

        // 파일 열기
        FILE* fp = fopen(fileName, "rb");
        if (fp == NULL) {
            printf("파일 입출력 오류");

        }

        // 파일 크기 보내기
        fseek(fp, 0, SEEK_END);
        filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        retval = send(sock, (char*)&filesize, sizeof(filesize), 0);


        //데이터 보내기(파일 데이터)
        //1KB씩 읽기

        while (numread = fread(buf, sizeof(char), BUFSIZE, fp)) {
            if (numread > 0) {
                SendMessage(hProgressBar, PBM_SETPOS, (int)(numtotal / filesize * 100), 0);
                // 데이터 보내기(파일 데이터)
                retval = send(sock, buf, numread, 0);

                numtotal += retval;
                Sleep(20);
            }
        }

        MessageBox(hWnd, L"파일 전송 성공!", L"전송 성공", MB_OK | MB_ICONINFORMATION);
    }
}