#include <iostream>
#include <fcntl.h>    // for open()
#include <unistd.h>   // for close()
#include <sys/ioctl.h>
#include <linux/fs.h> // for BLKGETSIZE64

int main() {
    // Requires sudo/root to open /dev/sda
    int fd = open("/dev/sda", O_RDONLY);
    if (fd == -1) {
        perror("Failed to open device");
        return 1;
    }

    uint64_t size_bytes = 0;
    if (ioctl(fd, BLKGETSIZE64, &size_bytes) == -1) {
        perror("ioctl failed");
        close(fd);
        return 1;
    }

    std::cout << "Total Size: " << size_bytes << " bytes" << std::endl;

    close(fd);
    return 0;
}
