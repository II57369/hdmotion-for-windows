#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <random>
#include <time.h>
#include <math.h>
#include <ShellAPI.h>

#define PI acos(-1)
#define M_PI		3.14159265358979323846

#define CALCULATE_LBA(f, MaxLBA, off_set) ((f * (MaxLBA - 1) + off_set <= MaxLBA) ? (f * (MaxLBA - 1)) + off_set : (f * (MaxLBA - 1)) - off_set)

#define READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead) \
    do { \
        LARGE_INTEGER sectorOffset; \
        sectorOffset.QuadPart = lbaAddress * 4096; \
        SetFilePointer(hDevice, sectorOffset.LowPart, &sectorOffset.HighPart, FILE_BEGIN); \
        if (!ReadFile(hDevice, buffer, sizeof(buffer), &bytesRead, NULL)) { \
            printf("Failed to read disk\n"); \
            exit(2); \
        } \
    } while(0)

// ԭ�� hdmotion for dos by Jeremy Stanley
// http://hdmotion.pingerthinger.com/hdmotion.zip
// adapted by 1157369
// ����ʾ����
// g++.exe ".\src.cpp" -o ".\src.exe" -std=c++11 -I"..\MinGW64\x86_64-w64-mingw32\include" -I"..\MinGW64\lib\gcc\x86_64-w64-mingw32\4.9.2\include" -I"..\MinGW64\lib\gcc\x86_64-w64-mingw32\4.9.2\include\c++" -L"..\MinGW64\lib" -L"..\MinGW64\x86_64-w64-mingw32\lib" -static-libgcc

using namespace std;

long long MaxLBA = -1;

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

void display(long long lbaAddress)
{
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

int readlba(DWORD diskNumber, long long lbaAddress)  // ���̺� + LBA��
{ // �������ѭ����ε��ûᵼ��CPUռ���ʸ� 
//	printf("reading %d...\n", lbaAddress);
    HANDLE hDevice;
    DWORD bytesRead;
    char buffer[4096];
    //DWORD diskNumber, lbaAddress;

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
	long long start1=0, start2=0, current;
    random_device rd;   // non-deterministic generator
    mt19937_64 gen(rd());  // to seed mersenne twister.
    uniform_int_distribution<long long> dist(0, MaxLBA); // distribute results between 1 and 6 inclusive.
    for (int i = 0; i < times; ++i) {
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
	    
        //��ӡ
		display(lbaAddress);
    }

    // �ر��������
    CloseHandle(hDevice);

    return 0;
}

int hdmotion(DWORD diskNumber)
{
//	printf("reading %d...\n", lbaAddress);
	long long cnt=0;
    HANDLE hDevice;
    DWORD bytesRead;
    char buffer[4096];

    // ���������
    char devicePath[32];
    sprintf(devicePath, "\\\\.\\PhysicalDrive%d", diskNumber);
    hDevice = CreateFile(devicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
    if (hDevice == INVALID_HANDLE_VALUE) 
	{
        printf("Failed to open device\n");
        exit(2);
    }


	long long lbaAddress=0;
	long long start1=0, start2=0, current;
    random_device rd;   // non-deterministic generator
    mt19937_64 gen(rd());  // to seed mersenne twister.
    uniform_int_distribution<long long> dist(0, MaxLBA/200/2); // distribute results between 1 and 6 inclusive.
	double f, s, l, h, amp;
	int i, heads;

	// accelerating zigzag
	s = 0.010;
	for(i = 0; i < 5; ++i) {
		for(f = 0.0; f < 1.0; f += s)
		{
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
		}
		for(f -= s; f > 0.0; f -= s)
		{
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
		}

		s += 0.0075;
		

	}
	f += s;
	
	
	// tightening zigzag
	h = 0.90;
	l = 0.10;
	for(; l < h;) {
		for(; f < h; f += s)
		{
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
		}
		for(; f > l; f -= s)
		{
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
		}

		h -= 0.05;
		l += 0.05;
	}


	// widening sinusoid
	amp = 0.05;
	for(; amp <= 0.50; amp += 0.05) {
		double x;
		for(x = 0; x < (2 * M_PI); x += M_PI / 32.0) {
			f = (sin(x) * amp) + 0.5;
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
		}
	}


	// narrowing sinusoid
	for(amp = 0.50; amp > 0.0; amp -= 0.05) {
		double x;
		for(x = 0; x < (2 * M_PI); x += M_PI / 32.0) {
			f = (sin(x) * amp) + 0.5;
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
		}
	}


	// widening double-sinusoid
	amp = 0.05;
	for(; amp <= 0.50; amp += 0.05) {
		double x;
		for(x = 0; x < (2 * M_PI); x += M_PI / 16.0) {
			f = (sin(x) * amp) + 0.5;
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
			
			f = 1.0 - f; //**********************************************************************************************************************************
			off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
		}
	}


	// narrowing double-sinusoid
	for(amp = 0.50; amp > 0.0; amp -= 0.05) {
		double x;
		for(x = 0; x < (2 * M_PI); x += M_PI / 16.0) {
			f = (sin(x) * amp) + 0.5;
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
			
			f = 1.0-f; //**********************************************************************************************************************************
			off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
		}
	}


	// buncha heads
	for(heads = 2; heads < 7; ++heads) {
		int repeat = 160 / heads;
		for(int i = 0; i < repeat; ++i) {
			for(int j = 1; j <= heads; ++j) {
				f = (double)j / (heads + 1);
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
			}
		}
	}
	for(; heads > 0; heads -= 2) {
		int repeat = 160 / heads;
		for(int i = 0; i < repeat; ++i) {
			for(int j = 1; j <= heads; ++j) {
				f = (double)j / (heads + 1);
			int off_set = dist(gen);
			lbaAddress = CALCULATE_LBA(f, MaxLBA, off_set);
			READ_DISK_DATA(hDevice, lbaAddress, buffer, bytesRead);
		    
	        //��ӡ
			display(lbaAddress);
			}
		}
	}


	// noise
	RandomSeek(diskNumber, 600);


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
        	printf("Unable to get administrator privileges, error code: %d\n", GetLastError());
            //std::cerr << "�޷���ȡ����ԱȨ�ޣ��������: " << GetLastError() << std::endl;
            return 1; // �˳�����
        }

        // ������������ԱȨ��
        return 0;
    } else {
        // �������߼�
        printf("The program has been run with administrator privileges.\n");
        //std::cout << "�������Թ���ԱȨ�����У�" << std::endl;
    }

    
	int drvnum, lba;
	printf("drive number:"); scanf("%d", &drvnum);
	MaxLBA = GetMaxLBAForDisk(drvnum);
	printf("Max LBA: %I64d\n", MaxLBA);
    
    //RandomSeek(drvnum, 1000000);
    hdmotion(drvnum);
    
    printf("Press enter to exit..."); getchar(); getchar(); //��һ�� getchar ���ȡ�� scanf �Ļس� 
	return 0;
}
