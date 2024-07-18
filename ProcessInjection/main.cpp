#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <iostream>


int main() {

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (!CreateProcess(L"C:\\Windows\\System32\\calc.exe", NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        printf("CreateProcess basarisiz.\n");
        return 1;
    }
    printf("CreateProcess basarili.\n");

    DWORD processId = pi.dwProcessId;

    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (!processHandle) {
        printf("OpenProcess basarisiz.\n");
        return 1;
    }
    printf("OpenProcess basarili.\n");


    const char* dllPath = "C:\\Users\\Parthenios\\Desktop\\injected.dll";
    LPVOID remoteMemory = VirtualAllocEx(processHandle, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!remoteMemory) {
        printf("VirtualAllocEx basarisiz.\n");
        CloseHandle(processHandle);
        return 1;
    }
    printf("VirtualAllocEx basarili.\n");

    if (!WriteProcessMemory(processHandle, remoteMemory, dllPath, strlen(dllPath) + 1, NULL)) {
        printf("WriteProcessMemory basarisiz.\n");
        VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }
    printf("WriteProcessMemory basarili.\n");

    FARPROC loadLibraryAddr = GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LoadLibraryA");
    if (!loadLibraryAddr) {
        printf("GetProcAddress basarisiz.\n");
        VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }
    printf("GetProcAddress basarili.\n");

    HANDLE remoteThread = CreateRemoteThread(processHandle, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, remoteMemory, 0, NULL);
    if (!remoteThread) {
        printf("CreateRemoteThread basarisiz.\n");
        VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
        CloseHandle(processHandle);
        return 1;
    }
    printf("CreateRemoteThread basarili.\n");

    WaitForSingleObject(remoteThread, INFINITE);
    printf("Remote thread tamamlandi.\n");

    DWORD exitCode;
    if (GetExitCodeThread(remoteThread, &exitCode)) {
        if (exitCode == 0) {
            printf("DLL yuklenmesi basarisiz.\n");
        }
        else {
            printf("DLL yuklenmesi basarili.\n");
        }
    }
    else {
        printf("GetExitCodeThread basarisiz.\n");
    }

    VirtualFreeEx(processHandle, remoteMemory, 0, MEM_RELEASE);
    CloseHandle(remoteThread);
    CloseHandle(processHandle);
    printf("Temizlik basarili.\n");

    return 0;
}
