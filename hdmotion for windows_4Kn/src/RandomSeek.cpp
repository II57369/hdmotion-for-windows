#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <random>
#include <time.h>
#include <math.h>
#include <ShellAPI.h>

// ԭ�� hdmotion for dos by by Jeremy Stanley
// http://hdmotion.pingerthinger.com/hdmotion.zip
// adapted by 1157369
// ����ʾ����
// g++.exe ".\src.cpp" -o ".\src.exe" -std=c++11 -I"..\MinGW64\x86_64-w64-mingw32\include" -I"..\MinGW64\lib\gcc\x86_64-w64-mingw32\4.9.2\include" -I"..\MinGW64\lib\gcc\x86_64-w64-mingw32\4.9.2\include\c++" -L"..\MinGW64\lib" -L"..\MinGW64\x86_64-w64-mingw32\lib" -static-libgcc

using namespace std;

// ����Ƿ�Ϊ����Ա������д˳��� 
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

    // ��ָ�����������
    char devicePath[32];
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", diskNumber);
    hDisk = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDisk == INVALID_HANDLE_VALUE) {
        printf("Failed to open device\n");
        exit(1);
    }

    // ��ȡ������̵ļ�����Ϣ
    if (!DeviceIoControl(hDisk, IOCTL_DISK_GET_DRIVE_GEOMETRY_EX, NULL, 0, &diskGeometry, sizeof(diskGeometry), &bytesReturned, NULL)) {
        printf("Failed to read disk\n");
        CloseHandle(hDisk);
        exit(1);
    }

    // �������LBA����
    maxLBA = diskGeometry.DiskSize.QuadPart / diskGeometry.Geometry.BytesPerSector - 1;

    // �ر�������̾��
    CloseHandle(hDisk);

    return maxLBA;
}

int readlba(DWORD diskNumber, long long lbaAddress)  // ���̺� + LBA��
{
//	printf("reading %d...\n", lbaAddress);
    HANDLE hDevice;
    DWORD bytesRead;
    char buffer[4096];
    //DWORD diskNumber, lbaAddress;

    // ��ȡ������̺ź�LBA��ַ
/*    printf("������������̺ţ�");
    scanf("%d", &diskNumber);
    printf("������LBA��ַ��");
    scanf("%d", &lbaAddress);
*/
    // ���������
    char devicePath[32];
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", diskNumber);
    hDevice = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
	{
        printf("Failed to open device\n");
        exit(2);
    }

    // ��ȡ����
    LARGE_INTEGER sectorOffset;
    sectorOffset.QuadPart = lbaAddress * 4096;
    SetFilePointer(hDevice, sectorOffset.LowPart, &sectorOffset.HighPart, FILE_BEGIN);
    if (!ReadFile(hDevice, buffer, sizeof(buffer), &bytesRead, NULL)) 
	{
        printf("Failed to read disk\n");
        exit(2);
    }
/*
    // �������
    for (int i = 0; i < bytesRead; i++) 
	{
        printf("%02X ", (unsigned char)buffer[i]);
        if ((i + 1) % 16 == 0) 
		{
            printf("\n");
        }
    }
*/
    // �ر��������
    CloseHandle(hDevice);

    return 0;
}

int RandomSeek(DWORD diskNumber, long long times)  // ���̺� + ��ȡ����
{
//	printf("reading %d...\n", lbaAddress);
	long long cnt=0;
    HANDLE hDevice;
    DWORD bytesRead;
    char buffer[4096];
    //DWORD diskNumber, lbaAddress;

    // ��ȡ������̺ź�LBA��ַ
/*    printf("������������̺ţ�");
    scanf("%d", &diskNumber);
    printf("������LBA��ַ��");
    scanf("%d", &lbaAddress);
*/
    // ���������
    char devicePath[32];
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", diskNumber);
    hDevice = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
	{
        printf("Failed to open device\n");
        exit(2);
    }

	// random seek
	time_t start, current;
    random_device rd;   // non-deterministic generator
    mt19937_64 gen(rd());  // to seed mersenne twister.
    uniform_int_distribution<long long> dist(0, GetMaxLBAForDisk(diskNumber)); // distribute results between 1 and 6 inclusive.
    for (long long i = 0; i < times || true; ++i) {
    	long long lbaAddress = dist(gen); // pass the generator to the distribution.
    	
	    // ��ȡ����
	    LARGE_INTEGER sectorOffset;
	    sectorOffset.QuadPart = lbaAddress * 4096;
	    SetFilePointer(hDevice, sectorOffset.LowPart, &sectorOffset.HighPart, FILE_BEGIN);
	    if (!ReadFile(hDevice, buffer, sizeof(buffer), &bytesRead, NULL)) 
		{
	        printf("Failed to read disk\n");
	        exit(2);
	    }
	    
	    //�ƴ�
		time(&start);
		if((current-start) != 0){
            printf("%d iops\n", cnt);
            current=start;
            cnt = 0;
        }
        cnt++;
    }


/*
    // �������
    for (int i = 0; i < bytesRead; i++) 
	{
        printf("%02X ", (unsigned char)buffer[i]);
        if ((i + 1) % 16 == 0) 
		{
            printf("\n");
        }
    }
*/
    // �ر��������
    CloseHandle(hDevice);

    return 0;
}

int main(){
    // ����Ƿ��Թ���ԱȨ������
    if (!IsRunAsAdministrator()) {
        // ��ȡ����·��
        char programPath[MAX_PATH];
        GetModuleFileName(NULL, programPath, MAX_PATH);

        // �������ԱȨ��
        if (ShellExecute(NULL, "runas", programPath, NULL, NULL, SW_SHOWNORMAL) <= (HINSTANCE)32) {
        	printf("�޷���ȡ����ԱȨ�ޣ��������: %d\n", GetLastError());
            //std::cerr << "�޷���ȡ����ԱȨ�ޣ��������: " << GetLastError() << std::endl;
            return 1; // �˳�����
        }

        // ������������ԱȨ��
        return 0;
    } else {
        // �������߼�
        printf("�������Թ���ԱȨ�����У�\n");
        //std::cout << "�������Թ���ԱȨ�����У�" << std::endl;
    }

    
	int drvnum, lba;
	printf("drive number:"); scanf("%d", &drvnum);
	printf("Max LBA: %I64d\n", GetMaxLBAForDisk(drvnum));
    
    RandomSeek(drvnum, 1000000);
    
	return 0;
}
