// 参考资料
// https://learn.microsoft.com/zh-cn/windows/win32/memory/creating-named-shared-memory
// https://learn.microsoft.com/zh-cn/windows/win32/api/memoryapi/nf-memoryapi-openfilemappingw
// https://learn.microsoft.com/zh-cn/windows/win32/api/memoryapi/nf-memoryapi-mapviewoffile

#include <windows.h>
#include <iostream>

using std::cin;
using std::cout;
using std::cerr;
using std::endl;

constexpr DWORD kBUFSIZE = 256;

int main() {
    std::wstring block_name = L"";
    cout << "Input the name of shared memory block: ";
    std::wcin >> block_name;
    block_name = L"Local\\" + block_name;

    HANDLE memory_block = OpenFileMappingW(
        FILE_MAP_READ,          //只读权限
        FALSE,                  //是否继承句柄，否
        block_name.c_str()      //共享内存名字
    );

    if (memory_block == NULL) {
        cerr << "Failed to open memory block" << endl;
        cerr << "Can be wrong block name" << endl;
        cerr << "Error code: " << GetLastError() << endl;
        system("pause");
        return 1;
    }

    HANDLE block_view = MapViewOfFile(
        memory_block,
        FILE_MAP_READ,
        0,
        0,
        kBUFSIZE
    );

    if (block_view == NULL) {
        cerr << "Fail to open shared memory block view, error code: " << GetLastError() << endl;
        CloseHandle(memory_block);
        system("pause");
        return 1;
    }

    std::cout << static_cast<const char *>(block_view) << endl;
    UnmapViewOfFile(block_view);
    CloseHandle(memory_block);
    system("pause");
    return 0;
}

/*
输出
Input the name of shared memory block: MyBlock
Hello there
请按任意键继续. . .
*/