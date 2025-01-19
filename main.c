#include <stdio.h>
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#include <winternl.h>
#include <powrprof.h>
#include <intrin.h>
#include <string.h>

#pragma comment(lib, "powrprof.lib")

typedef BOOL (WINAPI *GLPI)(PSYSTEM_LOGICAL_PROCESSOR_INFORMATION, PDWORD);

// Helper function to count set bits in the processor mask
static DWORD CountSetBits(ULONG_PTR bitMask) {
    DWORD LSHIFT = sizeof(ULONG_PTR) * 8 - 1;
    DWORD bitSetCount = 0;
    for (DWORD i = 0; i <= LSHIFT; i++) {
        bitSetCount += ((bitMask >> i) & 1);
    }
    return bitSetCount;
}

// Function to clear console screen
void clearScreen() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD coord = {0, 0};
    DWORD count;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    FillConsoleOutputCharacter(hStdOut, ' ', csbi.dwSize.X * csbi.dwSize.Y, coord, &count);
    SetConsoleCursorPosition(hStdOut, coord);
}

// Function to get CPU features using CPUID
void getCPUFeatures() {
    int cpuInfo[4] = {-1};
    char vendor[13];
    
    // Get vendor string
    __cpuid(cpuInfo, 0);
    memcpy(vendor, &cpuInfo[1], 4);
    memcpy(vendor + 4, &cpuInfo[3], 4);
    memcpy(vendor + 8, &cpuInfo[2], 4);
    vendor[12] = '\0';
    
    printf("\nCPU Features:\n");
    printf("--------------\n");
    printf("Vendor ID: %s\n", vendor);

    // Get features
    __cpuid(cpuInfo, 1);
    printf("Stepping ID: %d\n", cpuInfo[0] & 0xF);
    printf("Model: %d\n", (cpuInfo[0] >> 4) & 0xF);
    printf("Family: %d\n", (cpuInfo[0] >> 8) & 0xF);
    printf("Processor Type: %d\n", (cpuInfo[0] >> 12) & 0x3);
    printf("Extended Model: %d\n", (cpuInfo[0] >> 16) & 0xF);
    printf("Extended Family: %d\n", (cpuInfo[0] >> 20) & 0xFF);

    // Feature flags
    printf("\nFeature Flags:\n");
    if (cpuInfo[3] & (1 << 0)) printf("FPU: Floating Point Unit on-chip\n");
    if (cpuInfo[3] & (1 << 23)) printf("MMX: MMX technology supported\n");
    if (cpuInfo[3] & (1 << 25)) printf("SSE: Streaming SIMD Extensions\n");
    if (cpuInfo[3] & (1 << 26)) printf("SSE2: Streaming SIMD Extensions 2\n");
    if (cpuInfo[2] & (1 << 0)) printf("SSE3: Streaming SIMD Extensions 3\n");
    if (cpuInfo[2] & (1 << 9)) printf("SSSE3: Supplemental Streaming SIMD Extensions 3\n");
    if (cpuInfo[2] & (1 << 19)) printf("SSE4.1: Streaming SIMD Extensions 4.1\n");
    if (cpuInfo[2] & (1 << 20)) printf("SSE4.2: Streaming SIMD Extensions 4.2\n");
    if (cpuInfo[2] & (1 << 28)) printf("AVX: Advanced Vector Extensions\n");
}

// Function to get CPU frequency information
void getCPUFrequency() {
    HKEY hKey;
    DWORD mhz;
    DWORD size = sizeof(DWORD);
    
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE)&mhz, &size) == ERROR_SUCCESS) {
            printf("\nCPU Frequency Information:\n");
            printf("-------------------------\n");
            printf("Current Frequency: %lu MHz\n", mhz);
        }
        RegCloseKey(hKey);
    }
}

// Function to print all CPU information
void printCPUInfo() {
    // Get basic system information
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    
    // Get system uptime
    DWORD uptime = GetTickCount64() / 1000;
    
    // Get process count
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry;
    processEntry.dwSize = sizeof(processEntry);
    int processCount = 0;
    
    if (Process32First(snapshot, &processEntry)) {
        do {
            processCount++;
        } while (Process32Next(snapshot, &processEntry));
    }
    CloseHandle(snapshot);

    // Get detailed CPU information
    GLPI glpi = (GLPI)GetProcAddress(GetModuleHandle(TEXT("kernel32")),
                                    "GetLogicalProcessorInformation");
    if (glpi != NULL) {
        DWORD returnLength = 0;
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = NULL;
        
        glpi(buffer, &returnLength);
        buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(returnLength);
        
        if (glpi(buffer, &returnLength)) {
            DWORD processorCoreCount = 0;
            DWORD logicalProcessorCount = 0;
            DWORD processorL1CacheCount = 0;
            DWORD processorL2CacheCount = 0;
            DWORD processorL3CacheCount = 0;
            DWORD processorPackageCount = 0;
            DWORD numaNodeCount = 0;
            
            PSYSTEM_LOGICAL_PROCESSOR_INFORMATION ptr = buffer;
            DWORD byteOffset = 0;
            
            while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
                switch (ptr->Relationship) {
                    case RelationProcessorCore:
                        processorCoreCount++;
                        logicalProcessorCount += CountSetBits(ptr->ProcessorMask);
                        break;
                    case RelationCache:
                        switch (ptr->Cache.Level) {
                            case 1:
                                processorL1CacheCount++;
                                break;
                            case 2:
                                processorL2CacheCount++;
                                break;
                            case 3:
                                processorL3CacheCount++;
                                break;
                        }
                        break;
                    case RelationProcessorPackage:
                        processorPackageCount++;
                        break;
                    case RelationNumaNode:
                        numaNodeCount++;
                        break;
                }
                byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
                ptr++;
            }

            printf("Detailed CPU Information:\n");
            printf("------------------------\n");
            printf("Processor Packages (Physical CPUs): %d\n", processorPackageCount);
            printf("NUMA Nodes: %d\n", numaNodeCount);
            printf("Processor Cores: %d\n", processorCoreCount);
            printf("Logical Processors (Threads): %d\n", logicalProcessorCount);
            printf("L1 Cache Count: %d\n", processorL1CacheCount);
            printf("L2 Cache Count: %d\n", processorL2CacheCount);
            printf("L3 Cache Count: %d\n", processorL3CacheCount);

            // Print cache details
            ptr = buffer;
            byteOffset = 0;
            printf("\nCache Information:\n");
            printf("-----------------\n");
            while (byteOffset + sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION) <= returnLength) {
                if (ptr->Relationship == RelationCache) {
                    printf("L%d Cache:\n", ptr->Cache.Level);
                    printf("  Size: %d KB\n", ptr->Cache.Size / 1024);
                    printf("  Type: ");
                    switch (ptr->Cache.Type) {
                        case CacheUnified:
                            printf("Unified\n");
                            break;
                        case CacheInstruction:
                            printf("Instruction\n");
                            break;
                        case CacheData:
                            printf("Data\n");
                            break;
                        case CacheTrace:
                            printf("Trace\n");
                            break;
                    }
                }
                byteOffset += sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
                ptr++;
            }
        }
        free(buffer);
    }

    // Get CPU features using CPUID
    getCPUFeatures();

    // Get CPU frequency information
    getCPUFrequency();

    // Get processor name from registry
    HKEY hKey;
    char processorName[256] = "Unknown";
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD size = sizeof(processorName);
        RegQueryValueEx(hKey, "ProcessorNameString", NULL, NULL, (LPBYTE)processorName, &size);
        RegCloseKey(hKey);
    }

    printf("\nSystem Information:\n");
    printf("------------------\n");
    printf("Processor Name: %s\n", processorName);
    printf("Architecture: ");
    switch(sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            printf("x64 (AMD or Intel)\n");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            printf("x86\n");
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            printf("ARM\n");
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            printf("ARM64\n");
            break;
        default:
            printf("Other\n");
    }
    printf("Page Size: %lu bytes\n", sysInfo.dwPageSize);
    printf("Minimum Application Address: 0x%p\n", sysInfo.lpMinimumApplicationAddress);
    printf("Maximum Application Address: 0x%p\n", sysInfo.lpMaximumApplicationAddress);
    printf("Active Processor Mask: 0x%llx\n", (unsigned long long)sysInfo.dwActiveProcessorMask);

    printf("\nPerformance Information:\n");
    printf("----------------------\n");
    printf("Processes Running: %d\n", processCount);
    printf("System Uptime: %lu seconds (%0.2f hours)\n", uptime, uptime/3600.0);

    // Get memory information
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&memInfo);
    printf("\nMemory Information:\n");
    printf("------------------\n");
    printf("Memory Load: %lu%%\n", memInfo.dwMemoryLoad);
    printf("Total Physical Memory: %.2f GB\n", (float)memInfo.ullTotalPhys/1073741824);
    printf("Available Physical Memory: %.2f GB\n", (float)memInfo.ullAvailPhys/1073741824);
}

void printUsage() {
    printf("Usage: wincpu [-c]\n");
    printf("Options:\n");
    printf("  -c    Run in cycle mode (continuous updates)\n");
    printf("  -h    Show this help message\n");
}

int main(int argc, char *argv[]) {
    BOOL cycleMode = FALSE;

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-c") == 0) {
            cycleMode = TRUE;
        } else if (strcmp(argv[i], "-h") == 0) {
            printUsage();
            return 0;
        } else {
            printf("Unknown option: %s\n", argv[i]);
            printUsage();
            return 1;
        }
    }

    if (cycleMode) {
        printf("Running in cycle mode. Press Ctrl+C to exit...\n\n");
        while(1) {
            clearScreen();
            printCPUInfo();
            Sleep(1000);
        }
    } else {
        printCPUInfo();
        printf("\nPress Enter to exit...");
        getchar();
    }

    return 0;
}
