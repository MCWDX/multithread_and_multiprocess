// 参考资料
// https://learn.microsoft.com/zh-cn/windows/win32/ipc/named-pipe-client
// https://yinjinjing.blog.csdn.net/article/details/79508568
// https://blog.csdn.net/Think88666/article/details/83717354
// https://cloud.tencent.com/developer/article/1625924?from=15425&frompage=seopage

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
    std::wstring pipe_name = L"\\\\.\\pipe\\test_pipe";
    pipe = CreateNamedPipeW(
        kPipeName,                              //管道名字
        PIPE_ACCESS_DUPLEX,                     //管道打开模式，DUPLEX是双向管道
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,    //管道模式，这里用的是读写，主进程用PIPE起头
        1,                                      //最大实例数
        kBufSize,                               //输出缓冲区大小
        kBufSize,                               //输入缓冲区大小
        1000,                                   //等待管道的默认时间值，单位是毫秒
        NULL                                    //安全属性，使用默认安全属性用NULL
    );
    if (pipe == INVALID_HANDLE_VALUE) {
        cerr << "Failed to create named pipe, error code: " << GetLastError() << endl;
        system("pause");
        return 1;
    }
    cout << "Wait for client" << endl;
    if (!ConnectNamedPipe(pipe, NULL)) {
        cerr << "Connection error, error code: " << GetLastError() << endl;
        system("pause");
        return 1;
    }
    cout << "Connected" << endl;
    
    char write_buf[kBufSize] = "Hello there.";
    char read_buf[kBufSize] = "";
    

    DWORD lpNumberOfBytesWritten;
    if (!WriteFile(pipe, write_buf, strlen(write_buf), &lpNumberOfBytesWritten, NULL)) {
        cerr << "Failed to write message to pipe, error code: " << GetLastError() << endl;
        cout << "Closing handle for failed to write" << endl;
        CloseHandle(pipe);
        system("pause");
        return 1;
    }

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
    
    CloseHandle(pipe);
    cout << "Pipe closed" << endl;

    system("pause");
    return 0;
}

/*
输出
Wait for client
Connected
Read bytes from client: General Kenobi!
Pipe closed
请按任意键继续. . .
*/