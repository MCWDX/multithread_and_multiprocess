// 父进程相关代码参考资料
// https://learn.microsoft.com/zh-cn/windows/win32/memory/creating-named-shared-memory
// https://learn.microsoft.com/zh-cn/windows/win32/api/memoryapi/nf-memoryapi-createfilemappingw
// https://learn.microsoft.com/zh-cn/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile
// https://www.juhe.cn/news/index/id/11121

#include <windows.h>
#include <thread>
#include <iostream>
#include <string>

using std::getline;
using std::string;
using std::cin;
using std::cout;
using std::cerr;
using std::endl;

constexpr DWORD kBufSize = 256;

int main() {
    
    std::wstring block_name = L"";
    cout << "Input name(or u can say it a ID) of shared memory block: ";
    getline(std::wcin, block_name);
    
    //一定要用Local\\或者Global\\起头，不然会报错，大小写敏感
    block_name = L"Local\\" + block_name;

    HANDLE memory_block = CreateFileMappingW(
        INVALID_HANDLE_VALUE,       //标记匿名映射用INVALID_HANDLE_VALUE，
        NULL,                       //安全属性，使用默认安全属性用NULL
        PAGE_READWRITE,             //对文件映射的权限，如读写，执行，复制等
        0,                          //最大大小的高位值
        kBufSize,                   //最大大小的低位值
        block_name.c_str()          //共享内存区的名字
    );

    if (memory_block == NULL) {
        cerr << "Failed to create shared memory block, error code: " << GetLastError() << endl;
        system("pause");
        return 1;
    }

    LPVOID block_view = MapViewOfFile(
        memory_block,               //文件映射的句柄
        FILE_MAP_ALL_ACCESS,        //文件访问权限
        0,                          //文件访问的字节偏移量高位值
        0,                          //文件访问的字节偏移量低位值
        kBufSize                    //映射区的大小
    );

    if (block_view == NULL) {
        cerr << "Fail to open shared memory block view, error code: " << GetLastError() << endl;
        CloseHandle(memory_block);
        system("pause");
        return 1;
    }

    string message;
    cout << "Input string that u wanna put into the shared memory block: ";
    std::getline(cin, message);

    if (message.length() >= 1) {
        //length() + 1是因为要把最后那个\0也写入
        if (memcpy_s(static_cast<void *>(block_view), kBufSize, message.c_str(), message.length() + 1)) {
            cerr << "Failed to copy message to shared memory block" << endl;
        }
        else {
        cout << "Message copied to shared memory block" << endl;
        cout << "Now sleep for 1 minutes to wait for sub process to read the message" << endl;
        std::this_thread::sleep_for(std::chrono::minutes(1));
        }
    } else {
        cout << "U input nothing, nothing is copied" << endl;
    }
    

    
    UnmapViewOfFile(block_view);
    CloseHandle(memory_block);
    cout << "shared memory block closed" << endl;
    system("pause");
    return 0;
}

/*
输出
Input name(or u can say it a ID) of shared memory block: MyBlock
Input string that u wanna put into the shared memory block: Hello there. General Kenobi!
Message copied to shared memory block
Now sleep for 1 minutes to wait for sub process to read the message
shared memory block closed
请按任意键继续. . .
*/