// 参考资料
// https://developer.aliyun.com/article/455282
// 

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#include <cstring>
#include <iostream>
#include <thread>

using std::cout;
using std::endl;

constexpr char name[] = "/shared_memory";
constexpr size_t kBufSize = 128;

int main() {
    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        cout << "Failed to create shared memory" << endl;
        return 1;
    } else {
        cout << "Shared memory created" << endl;
    }

    if (ftruncate(shm_fd, kBufSize) == -1) {
        cout << "Failed to resize shared memory" << endl;
        return 1;
    } else {
        cout << "Shared memory resized" << endl;
    }

    void* shm_ptr = mmap(nullptr, kBufSize, PROT_WRITE | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        cout << "Failed to map shared memory" << endl;
    }

    memset(shm_ptr, 0, kBufSize );

    const char message[] = "Hello there";

    strncpy(static_cast<char *>(shm_ptr), message, strlen(message) + 1);

    std::this_thread::sleep_for(std::chrono::seconds(5));

    if (munmap(shm_ptr, kBufSize) == -1) {
        cout << "Failed to unmap shared memory" << endl;
        return 1;
    } else {
        cout << "Shared memory unmapped" << endl;
    }

    if (shm_unlink(name) == -1) {
        cout << "Failed to delete shared memory" << endl;
    } else {
        cout << "Shared memory deleted" << endl;
    }

    if (close(shm_fd) == -1) {
        cout << "Failed to close file descriptor" << endl;
    }
    return 0;
}