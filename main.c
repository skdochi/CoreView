// This is my first C Project v0.1 till v0.9 is going to remain in terminal ill start working on GUI in version v1.0,
// There are many projects similar to this like my main inspiration from CrystalDiskInfo but i'll work on making this one include everything possible !!
// Thank you for reading this, underneath is the source code with some comments to explain functions (mainly procedures).
//@skdochi on GitHub
// https://github.com/skdochi
//-This project uses actual Windows Interfaces (Windows APIs) : Registry
// WMI, DXGI, and IP Helper API
//---------------------------------------------------------------------------//
//includes !! (Libraries & Header files)
//-Standard C Files
#include <stdio.h>  //input and output functions (printf, scanf, etc...)
#include <string.h> // string functions
#include <stdlib.h> // General utilities and memory management
#include <stdarg.h> // Support for functions with variable number of arguments

// Windows networking Libraries
#include <winsock2.h> // Windows Sockets API
#include <ws2tcpip.h> // Additional TCP/IP networking functions

// Windows system API
#include <windows.h>  // Windows API : Let's C interact with windows system

// DXGI configuration for GPU information
#define COBJMACROS
#define CINTERFACE
#include <dxgi.h> // DirectX Graphics Infrastructure: GPU adapter information

// Windows Management Instrumentation (WMI)
#include <wbemidl.h> // Access to Windows hardware and system information
#include <oleauto.h> // Automation functions, including BSTR and VARIANT handling

// Windows network adapter information
#include <iphlpapi.h> // Retrieves information about network adapters and IP addresses
//---------------------------------------------------------------------------// 
// All of the procedures used in 'int main(void) {call; return0;}'
void printWmiString(
    IWbemClassObject *object,
    const wchar_t *property,
    const char *label
);

// INDICATORS 

// UNUSED CODE BLOCK : 
// Also i will add in the future customized GUI
//WORD TITLE_COLOR;
//WORD LABEL_COLOR;
//WORD VALUE_COLOR;
// WORD NORMAL_COLOR;
// Configuring all colors here manually
// (fun to mess with if you are reading this source code)
// UNUSED CODE BLOCK :
// void configureColors(void)
//{
//    TITLE_COLOR  = 9;   // Bright blue 9
//    LABEL_COLOR  = 12;  // Bright red 12
//    VALUE_COLOR  = 14;  // Bright yellow 14
//    NORMAL_COLOR = 7;   // gray 7
//}
// Replace the unused code block with this (trying to have more colors)
#define TITLE_COLOR  "\033[38;2;102;217;239m"  // Cyan #66D9EF
#define LABEL_COLOR  "\033[38;2;166;226;46m"   // Green #A6E22E
#define VALUE_COLOR  "\033[38;2;249;38;114m"   // Pink #F92672
#define NORMAL_COLOR "\033[38;2;169;169;169m"  // Gray #A9A9A9
#define RESET_COLOR  "\033[0m"
// Changes the console text color using a windows console attribute
void setColor(const char *color)
{
    printf("%s", color);
}

void resetColor(void)
{
    printf("%s", RESET_COLOR);
}
//ANSI colors in windows 'enabling'
void enableANSIColors(void)
{
    HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

    DWORD mode;

    GetConsoleMode(console, &mode);

    SetConsoleMode(
        console,
        mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
    );
}
// Prints formatted text using coreview's custom console output system (can be changed)
// handles color formatting before displaying the actual messages on the console 
// can also be changed but might cause bugs
int corePrintf(const char *format, ...)
{
    char buffer[4096];
    va_list args;
    va_start(args, format);
    int length = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    if (length < 0)
        return length;
    buffer[sizeof(buffer) - 1] = '\0';
    char *line = buffer;
    while (*line)
    {
        char *end = strchr(line, '\n');
        size_t lineLength = end
            ? (size_t)(end - line)
            : strlen(line);
        char currentLine[4096];

        if (lineLength >= sizeof(currentLine))
            lineLength = sizeof(currentLine) - 1;
        memcpy(currentLine, line, lineLength);
        currentLine[lineLength] = '\0';
        char *colon = strchr(currentLine, ':');
        if (colon != NULL)
        {
            char *content = colon + 1;
            while (*content == ' ')
                content++;
            if (*content == '\0')
            {
                setColor(TITLE_COLOR);
                fwrite(currentLine, 1, lineLength, stdout);
            }
            else
            {
                setColor(LABEL_COLOR);
                fwrite(currentLine, 1,
                        (size_t)(colon - currentLine + 1), stdout);
                setColor(VALUE_COLOR);
                fwrite(colon + 1, 1,
                        lineLength - (size_t)(colon + 1 - currentLine),
                        stdout);
            }
        }
        else if (strstr(currentLine, "====") != NULL ||
                    strstr(currentLine, "----") != NULL)
        {
            setColor(NORMAL_COLOR);
            fwrite(currentLine, 1, lineLength, stdout);
        }
        else if (currentLine[0] == ' ')
        {
            setColor(VALUE_COLOR);
            fwrite(currentLine, 1, lineLength, stdout);
        }
        else
        {
            setColor(TITLE_COLOR);
            fwrite(currentLine, 1, lineLength, stdout);
        }
        resetColor();
        if (end)
        {
            putchar('\n');
            line = end + 1;
        }
        else
        {
            break;
        }
    }
    return length;
}
#define printf corePrintf
// Detects and displays the processor architecture
void displayArchitecture(void)
{
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    printf(" System Architecture:\n");
    switch (info.wProcessorArchitecture)
    {
        case PROCESSOR_ARCHITECTURE_AMD64:
            printf(" x64 (64-bit)\n");
            break;

        case PROCESSOR_ARCHITECTURE_INTEL:
            printf(" x86 (32-bit)\n");
            break;

        case PROCESSOR_ARCHITECTURE_ARM64:
            printf(" ARM64\n");
            break;

        default:
            printf(" Unknown architecture\n");
            break;
    }
}
//Displays CPU Model name from the Windows Registery
void displayCPU(void)
{
    HKEY hKey;
    char cpuName[256] = {0};

    // Stores the size of the buffer.
    DWORD bufferSize = sizeof(cpuName);
    DWORD dataType;

    // Open the Windows Registery key containing CPU information
    LONG result = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0,
        KEY_READ,
        &hKey
    );

    if (result == ERROR_SUCCESS)
    {
        // Retrieve the ProcessorNameString value from the Registry
        result = RegQueryValueExA(
            hKey,
            "ProcessorNameString",
            NULL,
            &dataType,
            (LPBYTE)cpuName,
            &bufferSize
        );

        if (result == ERROR_SUCCESS)
        {
            // Ensure the string ends safely within the buffer
            cpuName[sizeof(cpuName) - 1] = '\0';
            printf(" %s\n", cpuName);
        }
        else
        {
            printf(" CPU information unavailable\n");
        }

        // Close the registry key after i'm done using it.
        RegCloseKey(hKey);
    }
    else
    {
        printf(" Unable to access CPU information\n");
    }
}
//Displays installed physical memory and calculates current RAM Usage
// (same as task manager but not LIVE Calculations in v0.2 i'll work on live statics)
void displayRAM(void)
{
    MEMORYSTATUSEX memory;

    // Windows requires the structure size before retrieving information.
    memory.dwLength = sizeof(memory);

    // Ask Windows to fill the structure with current memory statistics.
    if (GlobalMemoryStatusEx(&memory))
    {
        
        double totalRAM = (double)memory.ullTotalPhys
                          / (1024.0 * 1024.0 * 1024.0);

        
        double availableRAM = (double)memory.ullAvailPhys
                              / (1024.0 * 1024.0 * 1024.0);


        double usedRAM = totalRAM - availableRAM;


        DWORD usage = memory.dwMemoryLoad;

        printf(" Total RAM:     %.2f GB\n", totalRAM);
        printf(" Used RAM:      %.2f GB\n", usedRAM);
        printf(" Available RAM: %.2f GB\n", availableRAM);
        printf(" RAM Usage:     %lu%%\n", (unsigned long)usage);
    }
    else
    {
        printf(" RAM information unavailable\n");
    }
}
// Calculates how long Windows has been running since the last system startup
void displayUptime(void)
{

    ULONGLONG uptime = GetTickCount64();


    ULONGLONG seconds = uptime / 1000;


    ULONGLONG days = seconds / 86400;


    seconds %= 86400;


    ULONGLONG hours = seconds / 3600;


    seconds %= 3600;


    ULONGLONG minutes = seconds / 60;

    printf(" System Uptime:\n");
    printf(" %llu Days, %llu Hours, %llu Minutes\n",
            days, hours, minutes);
}
// Reads Windows version information from the Registry,
// including the edition, version number, and build number
void displayWindowsVersion(void)
{
    HKEY hKey;

    char productName[128] = {0};
    char displayVersion[64] = {0};
    char buildNumber[32] = {0};

    DWORD bufferSize;
    DWORD dataType;

    LONG result = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0,
        KEY_READ,
        &hKey
    );

    if (result == ERROR_SUCCESS)
    {

        bufferSize = sizeof(productName);

        if (RegQueryValueExA(
            hKey, "ProductName", NULL,
            &dataType, (LPBYTE)productName, &bufferSize
        ) == ERROR_SUCCESS)
        {
            productName[sizeof(productName) - 1] = '\0';
        
DWORD build = 0;
bufferSize = sizeof(buildNumber);
if (RegQueryValueExA(
    hKey, "CurrentBuildNumber", NULL,
    &dataType, (LPBYTE)buildNumber, &bufferSize
) == ERROR_SUCCESS)
{
    build = (DWORD)atoi(buildNumber);

    if (build >= 22000)
        printf(" Operating System: Windows 11\n");
    else
        printf(" Operating System: Windows 10\n");
}
else
{
    printf(" Operating System: %s\n", productName);
}
        }
        else
        {
            printf(" Operating System: Unavailable\n");
        }

        bufferSize = sizeof(displayVersion);

        if (RegQueryValueExA(
            hKey, "DisplayVersion", NULL,
            &dataType, (LPBYTE)displayVersion, &bufferSize
        ) == ERROR_SUCCESS)
        {
            displayVersion[sizeof(displayVersion) - 1] = '\0';
            printf(" Windows Version: %s\n", displayVersion);
        }
        else
        {
            printf(" Windows Version: Unavailable\n");
        }


        bufferSize = sizeof(buildNumber);

        if (RegQueryValueExA(
            hKey, "CurrentBuildNumber", NULL,
            &dataType, (LPBYTE)buildNumber, &bufferSize
        ) == ERROR_SUCCESS)
        {
            buildNumber[sizeof(buildNumber) - 1] = '\0';
            printf(" Build Number: %s\n", buildNumber);
        }
        else
        {
            printf(" Build Number: Unavailable\n");
        }

        RegCloseKey(hKey);
    }
    else
    {
        printf(" Windows information unavailable\n");
    }
}
// Retrieves the number of logical processors and physical CPU cores
void displayProcessorCores(void)
{
    SYSTEM_INFO info;


    GetSystemInfo(&info);
    printf(" Logical Processors: %lu\n",
            (unsigned long)info.dwNumberOfProcessors);
}
// Enumerates available display devices and displays their GPU names
void displayGPU(void)
{
    DISPLAY_DEVICEA device;
    char previousGPU[256] = "";
    DWORD i = 0;
    int found = 0;

    printf(" Graphics:\n");

    while (1)
    {
        ZeroMemory(&device, sizeof(device));
        device.cb = sizeof(device);

        if (!EnumDisplayDevicesA(NULL, i, &device, 0))
            break;

        if (!(device.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER))
        {

            if (strcmp(device.DeviceString, previousGPU) != 0)
            {
                printf(" %s\n", device.DeviceString);


                strcpy(previousGPU, device.DeviceString);

                found = 1;
            }
        }

        i++;
    }

    if (!found)
    {
        printf(" GPU information unavailable\n");
    }
}
// Retrieves graphics memory information from the available GPU adapters
void displayGPU_Memory(void)
{
    IDXGIFactory1 *factory = NULL;
    IDXGIAdapter1 *adapter = NULL;

    DXGI_ADAPTER_DESC1 description;

    UINT i = 0;
    HRESULT result;
    char gpuName[256];

    printf(" GPU Memory:\n");

    result = CreateDXGIFactory1(
        &IID_IDXGIFactory1,
        (void **)&factory
    );

    if (FAILED(result))
    {
        printf(" GPU memory information unavailable\n");
        return;
    }

    while (IDXGIFactory1_EnumAdapters1(factory, i, &adapter) != DXGI_ERROR_NOT_FOUND)
    {
        if (adapter == NULL)
            break;

        result = IDXGIAdapter1_GetDesc1(adapter, &description);

        if (SUCCEEDED(result))
        {
            if (!(description.Flags & DXGI_ADAPTER_FLAG_SOFTWARE))
            {

                WideCharToMultiByte(
                    CP_UTF8, 0,
                    description.Description, -1,
                    gpuName, sizeof(gpuName),
                    NULL, NULL
                );

        
                unsigned long long dedicated =
                    description.DedicatedVideoMemory / (1024ULL * 1024ULL);

        
                unsigned long long shared =
                    description.SharedSystemMemory / (1024ULL * 1024ULL);

                printf(" %s:\n", gpuName);
                printf("   Dedicated: %llu MB\n", dedicated);
                printf("   Shared:    %llu MB\n", shared);
            }
        }

        IDXGIAdapter1_Release(adapter);
        adapter = NULL;
        i++;
    }

    IDXGIFactory1_Release(factory);
}
// Enumerates fixed drives and displays their total and available storage
void displayDiskStorage(void)
{
    DWORD drives = GetLogicalDrives();
    char drivePath[] = "A:\\";

    ULARGE_INTEGER freeBytes;
    ULARGE_INTEGER totalBytes;
    ULARGE_INTEGER totalFreeBytes;

    printf(" Disk Storage:\n");

    if (drives == 0)
    {
        printf(" Disk information unavailable\n");
        return;
    }


    for (int i = 0; i < 26; i++)
    {
        if (drives & (1 << i))
        {
            drivePath[0] = 'A' + i;

            if (GetDriveTypeA(drivePath) == DRIVE_FIXED)
            {
                if (GetDiskFreeSpaceExA(
                        drivePath,
                        &freeBytes,
                        &totalBytes,
                        &totalFreeBytes))
                {
                
                    double total = (double)totalBytes.QuadPart
                                   / (1024.0 * 1024.0 * 1024.0);

                    double available = (double)totalFreeBytes.QuadPart
                                       / (1024.0 * 1024.0 * 1024.0);

                    double used = total - available;

                    printf("\n Drive %c:\n", 'A' + i);
                    printf("   Total:     %.2f GB\n", total);
                    printf("   Used:      %.2f GB\n", used);
                    printf("   Available: %.2f GB\n", available);
                }
            }
        }
    }
}
// Retrieves network adapter information using the Windows IP Helper API
// Displays available adapter details such as names and network addresses
void displayNetwork(void)
{
    ULONG bufferSize = 0;


    GetAdaptersAddresses(
        AF_INET,
        GAA_FLAG_INCLUDE_GATEWAYS,
        NULL,
        NULL,
        &bufferSize
    );

    IP_ADAPTER_ADDRESSES *adapters = malloc(bufferSize);

    if (adapters == NULL)
    {
        printf(" Network information unavailable\n");
        return;
    }


    ULONG result = GetAdaptersAddresses(
        AF_INET,
        GAA_FLAG_INCLUDE_GATEWAYS,
        NULL,
        adapters,
        &bufferSize
    );

    if (result != NO_ERROR)
    {
        printf(" Network information unavailable\n");
        free(adapters);
        return;
    }

    printf(" Network:\n");

    int found = 0;

    for (IP_ADAPTER_ADDRESSES *adapter = adapters;
            adapter != NULL;
            adapter = adapter->Next)
    {

        if (adapter->OperStatus != IfOperStatusUp ||
            adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK)
            continue;

        char adapterName[256];

        WideCharToMultiByte(
            CP_UTF8, 0,
            adapter->FriendlyName, -1,
            adapterName, sizeof(adapterName),
            NULL, NULL
        );

        printf(" Adapter: %s\n", adapterName);

        
        int hasIPv4 = 0;

        for (IP_ADAPTER_UNICAST_ADDRESS *address =
                    adapter->FirstUnicastAddress;
                address != NULL;
            address = address->Next)
        {
            if (address->Address.lpSockaddr->sa_family == AF_INET)
            {
                struct sockaddr_in *ipv4 =
                    (struct sockaddr_in *)address->Address.lpSockaddr;

                char ip[INET_ADDRSTRLEN];

                if (InetNtopA(
                    AF_INET, &ipv4->sin_addr,
                    ip, sizeof(ip)) != NULL)
                {
                    printf(" IPv4 Address: %s\n", ip);
                    hasIPv4 = 1;
                }
            }
        }

        if (!hasIPv4)
            printf(" IPv4 Address: Unavailable\n");

    
        if (adapter->FirstGatewayAddress != NULL)
        {
            struct sockaddr_in *gateway =
                (struct sockaddr_in *)
                adapter->FirstGatewayAddress->Address.lpSockaddr;

            char gatewayIP[INET_ADDRSTRLEN];

            if (gateway->sin_family == AF_INET &&
                InetNtopA(
                    AF_INET, &gateway->sin_addr,
                    gatewayIP, sizeof(gatewayIP)) != NULL)
            {
                printf(" Default Gateway: %s\n", gatewayIP);
            }
        }
        else
        {
            printf(" Default Gateway: Unavailable\n");
        }

        

        if (adapter->PhysicalAddressLength > 0)
        {
            char macStr[64] = {0};
            size_t offset = 0;

            for (ULONG i = 0; i < adapter->PhysicalAddressLength; i++)
            {
                offset += snprintf(
                    macStr + offset,
                    sizeof(macStr) - offset,
                    "%02X%s",
                    adapter->PhysicalAddress[i],
                    (i < adapter->PhysicalAddressLength - 1) ? "-" : ""
                );
            }

            printf(" MAC Address: %s\n", macStr);
        }
        else
        {
            printf(" MAC Address: Unavailable\n");
        }

        printf("\n");
        found = 1;
    }

    if (!found)
        printf(" No active network adapters found\n");

    free(adapters);
}
// Retrieves the current battery status and reports its charge level and state
void displayBattery(void)
{
    SYSTEM_POWER_STATUS battery;

    if (GetSystemPowerStatus(&battery))
    {
        printf(" Battery:\n");

        
        if (battery.BatteryFlag == 128)
        {
            printf(" No battery detected\n");
            return;
        }

        if (battery.BatteryLifePercent != 255)
        {
            printf(" Charge: %u%%\n",
                    (unsigned int)battery.BatteryLifePercent);
        }
        else
        {
            printf(" Charge: Unavailable\n");
        }


        if (battery.BatteryFlag & 8)
            printf(" Status: Charging\n");
        else if (battery.ACLineStatus == 1)
            printf(" Status: Plugged in\n");
        else if (battery.ACLineStatus == 0)
            printf(" Status: Discharging\n");
        else
            printf(" Status: Unknown\n");
    }
    else
    {
        printf(" Battery information unavailable\n");
    }
}
// Executes a WMI query and retrieves the requested Windows management object
// Used by other procedures to access hardware and system information
IWbemClassObject *getWmiObject(
    IWbemServices *service,
    const wchar_t *queryText)
{
    IEnumWbemClassObject *enumerator = NULL;
    IWbemClassObject *object = NULL;

    BSTR language = SysAllocString(L"WQL");
    BSTR query = SysAllocString(queryText);

    ULONG returned = 0;

    HRESULT result = IWbemServices_ExecQuery(
        service,
        language,
        query,
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &enumerator
    );

    SysFreeString(language);
    SysFreeString(query);

    if (SUCCEEDED(result))
    {
        IEnumWbemClassObject_Next(
            enumerator,
            WBEM_INFINITE,
            1,
            &object,
            &returned
        );
    }

    if (enumerator != NULL)
        IEnumWbemClassObject_Release(enumerator);

    return object;
}
// Uses Windows Management Instrumentation (WMI) to retrieve motherboard details
void displayMotherboard(void)
{
    IWbemLocator *locator = NULL;
    IWbemServices *service = NULL;

    IWbemClassObject *board = NULL;
    IWbemClassObject *bios = NULL;

    HRESULT result;

    result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(result))
    {
        printf(" Motherboard information unavailable\n");
        return;
    }

    result = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL
    );

    if (FAILED(result) && result != RPC_E_TOO_LATE)
        goto cleanup;

    result = CoCreateInstance(
        &CLSID_WbemLocator,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_IWbemLocator,
        (void **)&locator
    );

    if (FAILED(result))
        goto cleanup;

    BSTR nameSpace = SysAllocString(L"ROOT\\CIMV2");

    result = IWbemLocator_ConnectServer(
    locator,
    nameSpace,
    NULL,
    NULL,
    NULL,
    0,
    NULL,
    NULL,
    &service
);

    SysFreeString(nameSpace);

    if (FAILED(result))
        goto cleanup;

    result = CoSetProxyBlanket(
        (IUnknown *)service,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(result))
        goto cleanup;


    board = getWmiObject(
        service,
        L"SELECT * FROM Win32_BaseBoard"
    );

    
    bios = getWmiObject(
        service,
        L"SELECT * FROM Win32_BIOS"
    );

    printf(" Motherboard:\n");

    if (board != NULL)
    {
        printWmiString(
            board, L"Manufacturer", "Manufacturer"
        );

        printWmiString(
            board, L"Product", "Model"
        );

        printWmiString(
            board, L"Version", "Version"
        );
    }
    else
    {
        printf(" Motherboard information unavailable\n");
    }

    printf(" BIOS:\n");

    if (bios != NULL)
    {
        printWmiString(
            bios, L"Manufacturer", "Manufacturer"
        );

        printWmiString(
            bios, L"SMBIOSBIOSVersion", "Version"
        );
    }
    else
    {
        printf(" BIOS information unavailable\n");
    }

cleanup:

    if (bios != NULL)
        IWbemClassObject_Release(bios);

    if (board != NULL)
        IWbemClassObject_Release(board);

    if (service != NULL)
        IWbemServices_Release(service);

    if (locator != NULL)
        IWbemLocator_Release(locator);

    CoUninitialize();
}
// Retrieves the computer manufacturer and model using WMI
void displayPCInfo(void)
{
    IWbemLocator *locator = NULL;
    IWbemServices *service = NULL;
    IWbemClassObject *computer = NULL;

    HRESULT result;

    result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(result))
    {
        printf(" PC information unavailable\n");
        return;
    }

    result = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL
    );

    if (FAILED(result) && result != RPC_E_TOO_LATE)
        goto cleanup;

    result = CoCreateInstance(
        &CLSID_WbemLocator,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_IWbemLocator,
        (void **)&locator
    );

    if (FAILED(result))
        goto cleanup;

    BSTR nameSpace = SysAllocString(L"ROOT\\CIMV2");

    result = IWbemLocator_ConnectServer(
    locator,
    nameSpace,
    NULL,
    NULL,
    NULL,
    0,
    NULL,
    NULL,
    &service
);

    SysFreeString(nameSpace);

    if (FAILED(result))
        goto cleanup;

    result = CoSetProxyBlanket(
        (IUnknown *)service,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(result))
        goto cleanup;

    computer = getWmiObject(
        service,
        L"SELECT * FROM Win32_ComputerSystem"
    );

    printf(" PC Information:\n");

    if (computer != NULL)
    {
        printWmiString(
            computer, L"Manufacturer", "Manufacturer"
        );

        printWmiString(
            computer, L"Model", "Model"
        );
    }
    else
    {
        printf(" PC information unavailable\n");
    }

cleanup:

    if (computer != NULL)
        IWbemClassObject_Release(computer);

    if (service != NULL)
        IWbemServices_Release(service);

    if (locator != NULL)
        IWbemLocator_Release(locator);

    CoUninitialize();
}
// Reads a string property from a WMI object and prints it with its label
void printWmiString(
    IWbemClassObject *object,
    const wchar_t *property,
    const char *label)
{
    VARIANT value;
    VariantInit(&value);

    HRESULT result = IWbemClassObject_Get(
        object, property, 0, &value, NULL, NULL
    );

    if (SUCCEEDED(result) && value.vt == VT_BSTR && value.bstrVal != NULL)
    {
        char text[256];

        WideCharToMultiByte(
            CP_UTF8, 0, value.bstrVal, -1,
            text, sizeof(text), NULL, NULL
        );

        printf(" %s: %s\n", label, text);
    }
    else
    {
        printf(" %s: Unavailable\n", label);
    }

    VariantClear(&value);
}
// Converts a WMI capacity value into a readable storage or memory size
//& Retrieves a numeric capacity value from a WMI object and displays it in mWh
void printWmiCapacity(
    IWbemClassObject *object,
    const wchar_t *property,
    const char *label)
{
    VARIANT value;
    VariantInit(&value);

    HRESULT result = IWbemClassObject_Get(
        object, property, 0, &value, NULL, NULL
    );

    if (SUCCEEDED(result))
    {
        unsigned long long capacity = 0;

        if (value.vt == VT_UI4)
            capacity = value.ulVal;
        else if (value.vt == VT_I4 && value.lVal >= 0)
            capacity = value.lVal;
        else
        {
            printf(" %s: Unavailable\n", label);
            VariantClear(&value);
            return;
        }

        printf(" %s: %llu mWh\n", label, capacity);
    }
    else
    {
        printf(" %s: Unavailable\n", label);
    }

    VariantClear(&value);
}
// Retrieves an unsigned numeric property from a WMI object
int getWmiUnsigned(
    IWbemClassObject *object,
    const wchar_t *property,
    unsigned long long *number)
{
    VARIANT value;
    VariantInit(&value);

    HRESULT result = IWbemClassObject_Get(
        object, property, 0, &value, NULL, NULL
    );

    if (FAILED(result))
    {
        VariantClear(&value);
        return 0;
    }

    switch (value.vt)
    {
        case VT_UI4:
            *number = value.ulVal;
            break;

        case VT_I4:
            if (value.lVal < 0)
            {
                VariantClear(&value);
                return 0;
            }
            *number = (unsigned long long)value.lVal;
            break;

        case VT_UI8:
            *number = value.ullVal;
            break;

        case VT_I8:
            if (value.llVal < 0)
            {
                VariantClear(&value);
                return 0;
            }
            *number = (unsigned long long)value.llVal;
            break;

        default:
            VariantClear(&value);
            return 0;
    }

    VariantClear(&value);
    return 1;
}
// Retrieves additional battery hardware information through WMI
void displayBatteryHardware(void)
{
    IWbemLocator *locator = NULL;
    IWbemServices *service = NULL;

    IWbemClassObject *battery = NULL;
    IWbemClassObject *capacity = NULL;

    HRESULT result;

    result = CoInitializeEx(NULL, COINIT_MULTITHREADED);

    if (FAILED(result))
    {
        printf(" Battery information unavailable\n");
        return;
    }

    result = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL
    );

    if (FAILED(result) && result != RPC_E_TOO_LATE)
        goto cleanup;

    result = CoCreateInstance(
        &CLSID_WbemLocator,
        NULL,
        CLSCTX_INPROC_SERVER,
        &IID_IWbemLocator,
        (void **)&locator
    );

    if (FAILED(result))
        goto cleanup;

    BSTR nameSpace = SysAllocString(L"ROOT\\WMI");

    result = IWbemLocator_ConnectServer(
    locator,
    nameSpace,
    NULL,
    NULL,
    NULL,
    0,
    NULL,
    NULL,
    &service
);

    SysFreeString(nameSpace);

    if (FAILED(result))
        goto cleanup;

    result = CoSetProxyBlanket(
        (IUnknown *)service,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(result))
        goto cleanup;

    battery = getWmiObject(
        service,
        L"SELECT * FROM BatteryStaticData"
    );

    capacity = getWmiObject(
        service,
        L"SELECT * FROM BatteryFullChargedCapacity"
    );

    printf(" Battery Information:\n");

    unsigned long long design = 0;
    unsigned long long full = 0;

    int hasDesign = 0;
    int hasFull = 0;

    if (battery != NULL)
    {
        printWmiString(
            battery, L"ManufacturerName", "Manufacturer"
        );

        printWmiString(
            battery, L"DeviceName", "Model"
        );

        printWmiString(
            battery, L"SerialNumber", "Serial Number"
        );

        hasDesign = getWmiUnsigned(
            battery, L"DesignedCapacity", &design
        );

        if (hasDesign)
        {
            printf(" Design Capacity: %llu mWh\n", design);
        }
        else
        {
            printf(" Design Capacity: Unavailable\n");
        }
    }
    else
    {
        printf(" Battery hardware information unavailable\n");
    }

    if (capacity != NULL)
    {
        hasFull = getWmiUnsigned(
            capacity, L"FullChargedCapacity", &full
        );

        if (hasFull)
        {
            printf(" Full Charge Capacity: %llu mWh\n", full);
        }
        else
        {
            printf(" Full Charge Capacity: Unavailable\n");
        }
    }
    else
    {
        printf(" Full Charge Capacity: Unavailable\n");
    }

    // battery health math calculation
    if (hasDesign && hasFull && design > 0)
    {
        double health = ((double)full / design) * 100.0;

        printf(" Battery Health: %.1f%%\n", health);
    }
    else
    {
        printf(" Battery Health: Unavailable\n");
    }

    goto cleanup;

cleanup:

    if (capacity != NULL)
        IWbemClassObject_Release(capacity);

    if (battery != NULL)
        IWbemClassObject_Release(battery);

    if (service != NULL)
        IWbemServices_Release(service);

    if (locator != NULL)
        IWbemLocator_Release(locator);

    CoUninitialize();
}
// Retrieves detailed CPU specifications through Windows Registry information
void displayCPUDetails() {

    HKEY key;
    char manufacturer[100] = "Unavailable";
    DWORD speed = 0;
    DWORD size;
    DWORD type;


    if (RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0,
        KEY_QUERY_VALUE,
        &key
    ) == ERROR_SUCCESS) {

        size = sizeof(manufacturer);

        RegQueryValueExA(
            key, "VendorIdentifier", NULL,
            &type, (BYTE*)manufacturer, &size
        );

        size = sizeof(speed);

        RegQueryValueExA(
            key, "~MHz", NULL,
            &type, (BYTE*)&speed, &size
        );

        RegCloseKey(key);
    }

    DWORD length = 0;
    GetLogicalProcessorInformation(NULL, &length);

    SYSTEM_LOGICAL_PROCESSOR_INFORMATION *buffer =
        malloc(length);

    DWORD physicalCores = 0;

    if (buffer != NULL &&
        GetLogicalProcessorInformation(buffer, &length)) {

        DWORD count = length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);

        for (DWORD i = 0; i < count; i++) {

            if (buffer[i].Relationship == RelationProcessorCore) {
                physicalCores++;
            }
        }
    }

    free(buffer);

    printf(" CPU Details:\n");
    printf(" Manufacturer: %s\n", manufacturer);
    printf(" Physical Cores: %lu\n", physicalCores);
    printf(" Reported Clock Speed: %lu MHz\n", speed);
}
// CoreView entry point & Procedures Call Ups
// Initializes the console and displays the collected system information
int main(void)
{ enableANSIColors();
    SetConsoleTitleA("CoreView v0.1 made by skdochi !!"); //Terminal Title Bar 
    char computername[MAX_COMPUTERNAME_LENGTH +1];
    //MAX_COMPUTER_LENGTH is defined by windows
    //GetComputerNameA is pre-defined function
    DWORD size = sizeof(computername);

    printf("|          CoreView v0.1            |\n");
    printf("|   Windows System Information      |\n");
    printf("|       Made By @skdochi !!         |\n");

    printf("\n-----------------------------------------------------------\n");
    printf(" Computer Name:\n");
    if (GetComputerNameA(computername, &size)) {
        printf(" %s\n", computername);
    } else {
        printf("Failed to retrieve computer name\n");
    }
    printf("-----------------------------------------------------------\n");
    displayArchitecture();
    printf("-----------------------------------------------------------\n");
    printf(" Processor:\n");
    displayCPU();
    displayCPUDetails();                                                                                // A LOT OF PROCEDURES CALLING
    displayProcessorCores();
    printf("-----------------------------------------------------------\n");
    printf(" Memory:\n");
    displayRAM();
    printf("-----------------------------------------------------------\n");
    displayUptime();
    printf("-----------------------------------------------------------\n");
    printf(" Operating System:\n");
    displayWindowsVersion();
    printf("-----------------------------------------------------------\n");
    displayGPU();
    printf("-----------------------------------------------------------\n");
    displayGPU_Memory();    
    printf("-----------------------------------------------------------\n");
    displayDiskStorage();
    printf("-----------------------------------------------------------\n"); 
    displayBattery();
    displayBatteryHardware();
    printf("-----------------------------------------------------------\n");
    displayNetwork();
    printf("-----------------------------------------------------------\n");
    displayMotherboard();
    printf("-----------------------------------------------------------\n");
    displayPCInfo();
    printf("-----------------------------------------------------------\n");
    printf("\nPress Enter to exit...");
    getchar();// Wait for PRESSING Enter before exiting
    return 0;
}
// use this command to rebuild after saving with CTRL + S : 
// gcc main.c coreview_res.o -o CoreView.exe -ladvapi32 -ldxgi -ldxguid -lole32 -loleaut32 -lwbemuuid -liphlpapi -lws2_32
// then shift + enter : .\CoreView.exe
// testing GIT CHANGES