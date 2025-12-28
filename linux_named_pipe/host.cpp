// reference
// https://cloud.tencent.com/developer/article/2473528
// https://blog.csdn.net/weixin_36299472/article/details/148567855


#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;
using std::ofstream;
using std::string;
using std::cout;
using std::endl;

int main() {
    fs::path current_path = fs::current_path();
    cout << current_path.c_str();
    const string path = "/root/桌面/Cpp_code/linux_named_pipe/tmp_fifo";
    if (mkfifo(path.c_str(), 0666) == - 1) {
        cout << "Failed to make fifo" << endl;
        return 1;
    }
    
    const string message = "This is a message";
    
    ofstream to_fifo(path, std::ios::out);
    
    if (!to_fifo) {
        cout << "Failed to open fifo" << endl;
    } else {
        to_fifo << message;
    }

    to_fifo.close();

    if (unlink(path.c_str()) == -1) {
        cout << "Failed to unlink fifo" << endl;
    } else {
        cout << "fifo unlinked" << endl;
    }

    return 0;
}
