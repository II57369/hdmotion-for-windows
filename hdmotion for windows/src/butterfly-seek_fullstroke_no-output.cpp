#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <random>
#include <time.h>
#include <math.h>
#include <ShellAPI.h>
#define PI acos(-1)

// 原作 hdmotion for dos by by Jeremy Stanley
// http://hdmotion.pingerthinger.com/hdmotion.zip
// adapted by 1157369
// 编译示例：
// g++.exe ".\src.cpp" -o ".\src.exe" -std=c++11 -I"..\MinGW64\x86_64-w64-mingw32\include" -I"..\MinGW64\lib\gcc\x86_64-w64-mingw32\4.9.2\include" -I"..\MinGW64\lib\gcc\x86_64-w64-mingw32\4.9.2\include\c++" -L"..\MinGW64\lib" -L"..\MinGW64\x86_64-w64-mingw32\lib" -static-libgcc

using namespace std;

long long MaxLBA = -1;

// 检测是否为管理员身份运行此程序 
bool IsRunAsAdministrator() {
    bool fIsRunAsAdmin = false;
    HANDLE hToken = NULL;

    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        TOKEN_ELEVATION elevation;
        DWORD cbSize = sizeof(TOKEN_ELEVATION);

        if (GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &cbSize)) {
            fIsRunAsAdmin = elevation.TokenIsElevated != 0;
        }
    }

    if (hToken) {
        CloseHandle(hToken);
    }

    return fIsRunAsAdmin;
}

DWORDLONG GetMaxLBAForDisk(int diskNumber) {
    HANDLE hDisk = INVALID_HANDLE_VALUE;
    DWORD bytesReturned = 0;
    DISK_GEOMETRY_EX diskGeometry;
    DWORDLONG maxLBA = 0;

    // 打开指定的物理磁盘
    char devicePath[32];
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", diskNumber);
    hDisk = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        printf("Failed to open device\n");
        exit(1);
    }

    // 获取物理磁盘的几何信息
    if (!DeviceIoControl(hDisk, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, NULL)) {
        printf("Failed to read disk\n");
        CloseHandle(hDisk);
        exit(1);
    }

    // 计算最大LBA数字
    maxLBA = diskGeometry.DiskSize.QuadPart / diskGeometry.Geometry.BytesPerSector - 1;

    // 关闭物理磁盘句柄
    CloseHandle(hDisk);

    return maxLBA;
}

void display(long long lbaAddress)
{
	return;
	double ratio = (double)lbaAddress/(MaxLBA+1);
	if(ratio>=1)
	{
		printf("LBA out of range!\n");
		exit(3);
	}
	putchar('|');
	for(int i=0; i<100; i++)
	{
		if(i == (int)(ratio*100))	putchar('#');
		else						putchar(' ');
	}
	putchar('|');putchar('\n');
}

int readlba(DWORD diskNumber, long long lbaAddress)  // 磁盘号 + LBA号
{
//	printf("reading %d...\n", lbaAddress);
    HANDLE hDevice;
    DWORD bytesRead;
    char buffer[512];
    //DWORD diskNumber, lbaAddress;

    // 获取物理磁盘号和LBA地址
/*    printf("请输入物理磁盘号：");
    scanf("%d", &diskNumber);
    printf("请输入LBA地址：");
    scanf("%d", &lbaAddress);
*/
    // 打开物理磁盘
    char devicePath[32];
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", diskNumber);
    hDevice = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
	{
        printf("Failed to open device\n");
        exit(2);
    }

    // 读取数据
    LARGE_INTEGER sectorOffset;
    sectorOffset.QuadPart = lbaAddress * 512;
    SetFilePointer(hDevice, sectorOffset.LowPart, &sectorOffset.HighPart, FILE_BEGIN);
    if (!ReadFile(hDevice, buffer, sizeof(buffer), &bytesRead, NULL)) 
	{
        printf("Failed to read disk\n");
        exit(2);
    }
/*
    // 输出数据
    for (int i = 0; i < bytesRead; i++) 
	{
        printf("%02X ", (unsigned char)buffer[i]);
        if ((i + 1) % 16 == 0) 
		{
            printf("\n");
        }
    }
*/
    // 关闭物理磁盘
    CloseHandle(hDevice);

    return 0;
}

int RandomSeek(DWORD diskNumber, long long times)  // 磁盘号 + 读取次数
{
//	printf("reading %d...\n", lbaAddress);
	long long cnt=0;
    HANDLE hDevice;
    DWORD bytesRead;
    char buffer[512];

    // 打开物理磁盘
    char devicePath[32];
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", diskNumber);
    hDevice = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
	{
        printf("Failed to open device\n");
        exit(2);
    }

	// random seek
	long long start1=0, start2=0, current;
    random_device rd;   // non-deterministic generator
    mt19937_64 gen(rd());  // to seed mersenne twister.
    uniform_int_distribution<long long> dist(0, MaxLBA); // distribute results between 1 and 6 inclusive.
    for (int i = 0; i < times; ++i) {
    	long long lbaAddress = dist(gen); // pass the generator to the distribution.
    	
	    // 读取数据
	    LARGE_INTEGER sectorOffset;
	    sectorOffset.QuadPart = lbaAddress * 512;
	    SetFilePointer(hDevice, sectorOffset.LowPart, &sectorOffset.HighPart, FILE_BEGIN);
	    if (!ReadFile(hDevice, buffer, sizeof(buffer), &bytesRead, NULL)) 
		{
	        printf("Failed to read disk\n");
	        exit(2);
	    }
	    
	    //计次
		current = clock();
		if((current-start1) >= 1000){
            printf("%d iops\n", cnt);
            start1 = current;
            cnt = 0;
        }
        cnt++;
        
        //打印
        display(lbaAddress);
	}
    // 关闭物理磁盘
    CloseHandle(hDevice);

    return 0;
}

int ButterflySeek(DWORD diskNumber, long long times)  // 磁盘号 + 读取次数 + 分辨率（每个周期需要读取几次） 
{
//	printf("reading %d...\n", lbaAddress);
	long long cnt=0;
    HANDLE hDevice;
    DWORD bytesRead;
    char buffer[512];

    // 打开物理磁盘
    char devicePath[32];
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", diskNumber);
    hDevice = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
	{
        printf("Failed to open device\n");
        exit(2);
    }

	// butterfly seek
	long long track=0;
	long long lbaAddress=0;
	long long start1=0, start2=0, current;
    random_device rd;   // non-deterministic generator
    mt19937_64 gen(rd());  // to seed mersenne twister.
    uniform_int_distribution<long long> dist(0, MaxLBA/100/2); // distribute results between 1 and 6 inclusive.
    for (int i = 0; i < times || true; ++i) {
    	long long off_set = dist(gen);
    	if(i%2==0)
    	{
    		lbaAddress = ((track+off_set <= MaxLBA) ? track+off_set : track-off_set); // pass the generator to the distribution.
		}
		else
		{
			lbaAddress = ((MaxLBA-track+off_set <= MaxLBA) ? MaxLBA-track+off_set : MaxLBA-track-off_set);
			//track = MaxLBA*sin(PI*i/2/Resolution)*sin(PI*i/2/Resolution);
		}
    	
	    // 读取数据
	    LARGE_INTEGER sectorOffset;
	    sectorOffset.QuadPart = lbaAddress * 512;
	    SetFilePointer(hDevice, sectorOffset.LowPart, &sectorOffset.HighPart, FILE_BEGIN);
	    if (!ReadFile(hDevice, buffer, sizeof(buffer), &bytesRead, NULL)) 
		{
	        printf("Failed to read disk\n");
	        exit(2);
	    }
	    
	    //计次
		current = clock();
		if((current-start1) >= 1000){
            printf("%d iops\n", cnt);
            start1 = current;
            cnt = 0;
        }
        cnt++;
        
        //打印
    	display(lbaAddress);
    }

    // 关闭物理磁盘
    CloseHandle(hDevice);

    return 0;
}

int main(){
    // 检查是否以管理员权限运行
    if (!IsRunAsAdministrator()) {
        // 获取程序路径
        char programPath[MAX_PATH];
        GetModuleFileName(NULL, programPath, MAX_PATH);

        // 请求管理员权限
        if (ShellExecute(NULL, "runas", programPath, NULL, NULL, SW_SHOWNORMAL) <= (HINSTANCE)32) {
        	printf("无法获取管理员权限，错误代码: %d\n", GetLastError());
            //std::cerr << "无法获取管理员权限，错误代码: " << GetLastError() << std::endl;
            return 1; // 退出程序
        }

        // 标记已请求管理员权限
        return 0;
    } else {
        // 程序主逻辑
        printf("程序已以管理员权限运行！\n");
        //std::cout << "程序已以管理员权限运行！" << std::endl;
    }

    
	int drvnum, lba;
	printf("drive number:"); scanf("%d", &drvnum);
	MaxLBA = GetMaxLBAForDisk(drvnum);
	printf("Max LBA: %I64d\n", MaxLBA);
    
    //RandomSeek(drvnum, 1000000);
    ButterflySeek(drvnum, 10000000);
	return 0;
}
