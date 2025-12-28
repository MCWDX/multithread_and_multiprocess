#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include <string>
#include <fstream>
#include <iostream>

using std::string;
using std::cout;
using std::endl;

int main() {
    const string path = "/root/桌面/Cpp_code/linux_named_pipe/tmp_fifo";
    
    std::ifstream from_fifo(path);

    if (!from_fifo) {
        cout << "Failed to open fifo" << endl;
        return 1;
    }

    string message;
    std::getline(from_fifo, message);
    cout << "message recieved: " << message << endl;

    return 0;
}