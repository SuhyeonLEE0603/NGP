#include "../../Common.h"

char* SERVERIP = (char*)"127.0.0.1";
#define SERVERPORT 9000
#define BUFSIZE    4096

HWND hEdit, hSendButton, hProgressBar;
char fileName[256];

// �Լ� ������ Ÿ��
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void OpenFileSelectionDialog(HWND hWnd);
void SendFiles(HWND hWnd, SOCKET sock);

// ������ ���� ����
int file_cnt{ 0 };

SOCKET sock;

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // ������ Ŭ���� ���
    const wchar_t CLASS_NAME[] = L"homework6";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    int retval;

    // ���� �ʱ�ȭ
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
        return 1;

    // ���� ����
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


    // ������ ����
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

    // ����Ʈ ��Ʈ�� ����
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

    // ���� ���� ��ư ����
    CreateWindowEx(
        0,
        L"BUTTON",
        L"���ϼ���",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        10, 220, 120, 30,
        hWnd,
        (HMENU)1,
        hInstance,
        NULL
    );

    // Send ��ư ����
    hSendButton = CreateWindowEx(
        0,
        L"BUTTON",
        L"������",
        WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        140, 220, 80, 30,
        hWnd,
        (HMENU)2,
        hInstance,
        NULL
    );

    // ���α׷��� �� �ʱ�ȭ

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

    // ������ ǥ��
    ShowWindow(hWnd, nCmdShow);

    // �޽��� ����
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // ���� ����
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

        // ���ϸ� ����
        TCHAR file_Name[256];
        ZeroMemory(&file_Name, sizeof(file_Name));
        lstrcpy(file_Name, PathFindFileName(ofn.lpstrFile));
        
        // ���ϸ� ���
        SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)L"���� : ");
        SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)file_Name);
        SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)L"\r\n");

        // wideString�� char�� ��ȯ
        int mbLen = WideCharToMultiByte(CP_ACP, 0, file_Name, -1, NULL, 0, NULL, NULL);
        char* mbString = new char[mbLen];
        WideCharToMultiByte(CP_ACP, 0, file_Name, -1, mbString, mbLen, NULL, NULL);

        // fileName�� ����
        lstrcpyA(fileName, mbString);
        ++file_cnt;
        delete[] mbString;
    }
    delete[] wideString;
}


void SendFiles(HWND hWnd, SOCKET sock) {


    if (file_cnt == 0) {
        MessageBox(hWnd, L"������ ���õ��� ����", L"���� ����", MB_OK | MB_ICONINFORMATION);
    }
    else {
        // ������ ��ſ� ����� ����
        ///////////////////////////////////////////////////
        int retval;
        char buf[BUFSIZE];
        int fileNameLen;					// ���ϸ� ����
        int filesize;						// ���� ũ��
        int numread;						// ���� ����Ʈ ��
        int numtotal = 0;					// �� ���� ������ ��
        ///////////////////////////////////////////////////

        // ������ ������(���ϸ� ����)
        fileNameLen = sizeof(fileName);
        TCHAR buffer[256];
        wsprintf(buffer, _T("���۵� ���ϸ��� ����: %u"), fileNameLen);
        MessageBox(hWnd, buffer, L"���� ����\n", MB_OK | MB_ICONINFORMATION);

        retval = send(sock, (char*)&fileNameLen, sizeof(fileNameLen), 0);

        // ������ ������(���ϸ�)
        retval = send(sock, (char*)fileName, fileNameLen * sizeof(TCHAR), 0);

        // ���� ����
        FILE* fp = fopen(fileName, "rb");
        if (fp == NULL) {
            printf("���� ����� ����");

        }

        // ���� ũ�� ������
        fseek(fp, 0, SEEK_END);
        filesize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        retval = send(sock, (char*)&filesize, sizeof(filesize), 0);


        //������ ������(���� ������)
        //1KB�� �б�

        while (numread = fread(buf, sizeof(char), BUFSIZE, fp)) {
            if (numread > 0) {
                SendMessage(hProgressBar, PBM_SETPOS, (int)(numtotal / filesize * 100), 0);
                // ������ ������(���� ������)
                retval = send(sock, buf, numread, 0);

                numtotal += retval;
                Sleep(20);
            }
        }

        MessageBox(hWnd, L"���� ���� ����!", L"���� ����", MB_OK | MB_ICONINFORMATION);
    }
}