// reference
// https://blog.csdn.net/weixin_36299472/article/details/148567855
// https://cloud.tencent.com.cn/developer/article/2435011
// https://cloud.tencent.com/developer/article/2507761?from=15425&frompage=seopage
// https://blog.csdn.net/2301_79695216/article/details/142107478

#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>

#include <cstring>
#include <filesystem>
#include <iostream>
#include <thread>

namespace fs = std::filesystem;
using std::cout;
using std::endl;

constexpr size_t kBufSize = 128;

int main() {
    key_t key = ftok(fs::current_path().c_str(), 1);
    const char message[] = "A message to be sent";
    if (key == -1) {
        cout << "Failed to generate key" << endl;
        return 1;
    } else {
        cout << "Key generated" << endl;
    }

    int shmid = shmget(key, kBufSize, IPC_CREAT | 0644);

    char* ptr = static_cast<char *>(shmat(shmid, nullptr, 0));
    if (ptr == (char *)(-1)) {
        cout << "Failed to get access to shared memory" << endl;
    }

    strncpy(ptr, message, strlen(message) + 1);
    std::this_thread::sleep_for(std::chrono::seconds(5));


    if (shmdt(static_cast<void *>(ptr)) == -1) {
        cout << "Failed to detach shared memory" << endl;
        return 1;
    }

    if (shmctl(shmid, IPC_RMID, nullptr) == -1) {
        cout << "Failed to delete shared memory" << endl;
    } else {
        cout << "Shared memory deleted" << endl;
    }
    
    return 0;
}