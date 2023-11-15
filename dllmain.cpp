#include "Proxy/proxy.h" // Include the proxy header
#include <Windows.h>
#include <iostream>
#include <vector>
#include <tlhelp32.h>
#include <thread>

HMODULE ourModule = 0; // Global variable to store the module handle

DWORD_PTR FindDMAAddy(HANDLE hProc, DWORD_PTR base, std::vector<unsigned int> offsets)
{
    DWORD_PTR addr = base;
    for (unsigned int offset : offsets)
    {
        if (!ReadProcessMemory(hProc, reinterpret_cast<BYTE*>(addr), &addr, sizeof(addr), NULL))
        {
            std::cerr << "Error in ReadProcessMemory: " << GetLastError() << std::endl;
            return 0; // Indicate failure
        }
        addr += offset;
    }
    return addr;
}

// New function to check the key state and set memory when pressed
void CheckKeyAndSetMemory() {
    static HANDLE hProcess = GetCurrentProcess();
    static DWORD_PTR moduleBase = (DWORD_PTR)GetModuleHandleW(L"teardown.exe");
    std::vector<unsigned int> offsets = { 0x50, 0x568 };
    DWORD_PTR boundaryAddr;

    while (ourModule != NULL) {
        if (GetAsyncKeyState(VK_F8) & 0x8000) { // F9
            boundaryAddr = FindDMAAddy(hProcess, moduleBase + 0x006F4F40, offsets);

            if (boundaryAddr != 0) {
                int newValue = 0;
                WriteProcessMemory(hProcess, reinterpret_cast<void*>(boundaryAddr), &newValue, sizeof(newValue), NULL);
                std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Prevent key spamming
                std::cout << "Boundary Removed!" << std::endl;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Polling delay
    }
}


void OUR_CODE()
{
    AllocConsole();
    FILE* pFile = nullptr;
    freopen_s(&pFile, "CONOUT$", "w", stdout);
    std::cout << R"(                                                                                                                
88888888ba,    88           88              88888888ba   88888888ba     ,ad8888ba,  8b        d8  8b        d8  
88      `"8b   88           88              88      "8b  88      "8b   d8"'    `"8b  Y8,    ,8P    Y8,    ,8P   
88        `8b  88           88              88      ,8P  88      ,8P  d8'        `8b  `8b  d8'      Y8,  ,8P    
88         88  88           88              88aaaaaa8P'  88aaaaaa8P'  88          88    Y88P         "8aa8"     
88         88  88           88              88""""""'    88""""88'    88          88    d88b          `88'      
88         8P  88           88              88           88    `8b    Y8,        ,8P  ,8P  Y8,         88       
88      .a8P   88           88              88           88     `8b    Y8a.    .a8P  d8'    `8b        88       
88888888Y"'    88888888888  88888888888     88           88      `8b    `"Y8888Y"'  8P        Y8       88       
                                                                                                                
                                                                                                                )" << std::endl;




}

bool IsTeardownProcess() {
    char processName[MAX_PATH] = { 0 };
    GetModuleFileNameA(nullptr, processName, MAX_PATH);

    // Extract the name of the executable from the path
    std::string fullPath(processName);
    std::string exeName = fullPath.substr(fullPath.find_last_of("\\/") + 1);
    return exeName == "teardown.exe";
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        ourModule = hModule;
        if (IsTeardownProcess()) {
            Proxy_Attach();
            OUR_CODE();
            std::thread keyThread(CheckKeyAndSetMemory);
            keyThread.detach(); // Detach the thread so it runs independently
        }
        break;

    case DLL_PROCESS_DETACH:
        ourModule = NULL; // Indicate the module is detaching
        if (IsTeardownProcess()) {
            Proxy_Detach();
        }
        break;
    }
    return TRUE;
}
