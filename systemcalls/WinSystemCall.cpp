#include <windows.h>
#include <winioctl.h>
#include <iostream>

int main() {
    // Requires Administrator privileges
    HANDLE hDevice = CreateFileW(L"\\\\.\\PhysicalDrive0", 
                                 0, FILE_SHARE_READ | FILE_SHARE_WRITE, 
                                 NULL, OPEN_EXISTING, 0, NULL);

    if (hDevice == INVALID_HANDLE_VALUE) {
        std::cerr << "Error: " << GetLastError() << std::endl;
        return 1;
    }

    GET_LENGTH_INFORMATION lengthInfo = {0};
    DWORD bytesReturned = 0;

    BOOL result = DeviceIoControl(hDevice, 
                                  IOCTL_DISK_GET_LENGTH_INFO, 
                                  NULL, 0, 
                                  &lengthInfo, sizeof(lengthInfo), 
                                  &bytesReturned, NULL);

    if (result) {
        std::cout << "Total Size: " << lengthInfo.Length.QuadPart << " bytes" << std::endl;
    } else {
        std::cerr << "DeviceIoControl failed: " << GetLastError() << std::endl;
    }

    CloseHandle(hDevice);
    return 0;
}
