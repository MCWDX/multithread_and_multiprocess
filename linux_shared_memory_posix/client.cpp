#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

using std::cout;
using std::endl;

constexpr char name[] = "/shared_memory";
constexpr size_t kBufSize = 128;

int main() {
    int shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        cout << "Failed to open shared memory" << endl;
        return 1;
    } else {
        cout << "Shared memory opened" << endl;
    }

    void* shm_ptr = mmap(nullptr, kBufSize, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        cout << "Failed to map shared memory" << endl;
        return -1;
    }

    char message[kBufSize] = "";
    strncpy(message, static_cast<char *>(shm_ptr), strlen(static_cast<char *>(shm_ptr)) + 1);
    cout << "got message: " << message << endl;

    munmap(shm_ptr, kBufSize);

    close(shm_fd);
    return 0;
}