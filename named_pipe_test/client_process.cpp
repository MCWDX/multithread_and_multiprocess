// 参考资料
// https://learn.microsoft.com/zh-cn/windows/win32/ipc/named-pipe-client
// https://learn.microsoft.com/zh-cn/windows/win32/api/fileapi/nf-fileapi-createfilew

#include <windows.h>
#include <iostream>
#include <string>
#include <thread>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

constexpr LPCWSTR kPipeName = L"\\\\.\\pipe\\test_pipe";
constexpr DWORD kBufSize = 256;

int main() {
    HANDLE pipe = NULL;
    pipe = CreateFileW(
        kPipeName,                      //文件名，此处用的管道名
        GENERIC_READ | GENERIC_WRITE,   //文件打开/创造权限
        0,                              //是否共享
        NULL,                           //安全属性，使用默认的NULL
        OPEN_EXISTING,                  //文件创建参数，此指打开现有文件
        FILE_ATTRIBUTE_NORMAL,          //文件属性，设为普通
        NULL
    );

    if (pipe == INVALID_HANDLE_VALUE) {
        cerr << "Failed to open named pipe" << endl;
        system("pause");
        return 1;
    }

    
    char read_buf[kBufSize] = "";
    char write_buf[kBufSize] = "General Kenobi!";

    int wait_count = 0;
    DWORD lpNumberOfBytesRead = 0;
    while (wait_count < 5) {
        if (!ReadFile(pipe, read_buf, kBufSize, &lpNumberOfBytesRead, NULL)) {
            cout << "Read nothing from pipe, wait_count: " << wait_count << "sleep for 50 milliseconds" << endl;
            wait_count++;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        } else {
            read_buf[lpNumberOfBytesRead] = 0;
            cout << "Read bytes from client: " << read_buf << endl;
            break;
        }
    }
    if (wait_count == 5) {
        cout << "Read nothing from client after waiting 5 times, stop reading" << endl;
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    DWORD lpNumberOfBytesWritten;
    if (!WriteFile(pipe, write_buf, strlen(write_buf), &lpNumberOfBytesWritten, NULL)) {
        cerr << "Failed to write message to pipe, error code: " << GetLastError() << endl;
        cout << "Closing handle for failed to write" << endl;
        CloseHandle(pipe);
        system("pause");
        return 1;
    }

    CloseHandle(pipe);
    cout << "handle closed" << endl;
    system("pause");
    return 0;
}

/*
输出
Read bytes from client: Hello there.
handle closed
请按任意键继续. . .
*/