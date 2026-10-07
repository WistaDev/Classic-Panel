// Classic Panel - launcher / injetor
// Fica em segundo plano e injeta classic_panel.dll nos processos alvo.

#include <windows.h>
#include <tlhelp32.h>
#include <set>
#include <string>

static const wchar_t *kTargets[] = {
    L"explorer.exe",
    L"control.exe",
    L"mblctr.exe",
    L"rundll32.exe",
    L"DpiScaling.exe",
};

static bool IsTarget(const wchar_t *exe)
{
    for (auto t : kTargets)
        if (_wcsicmp(exe, t) == 0)
            return true;
    return false;
}

static bool AlreadyInjected(DWORD pid, const std::wstring &dllName)
{
    HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snap == INVALID_HANDLE_VALUE)
        return false;
    MODULEENTRY32W me = { sizeof(me) };
    bool found = false;
    if (Module32FirstW(snap, &me))
    {
        do
        {
            if (_wcsicmp(me.szModule, dllName.c_str()) == 0) { found = true; break; }
        } while (Module32NextW(snap, &me));
    }
    CloseHandle(snap);
    return found;
}

static bool Inject(DWORD pid, const std::wstring &dllPath)
{
    HANDLE hProc = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION |
                               PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ,
                               FALSE, pid);
    if (!hProc)
        return false;

    // Só injeta em processos 64 bits (a DLL é x64)
    BOOL wow64 = FALSE;
    IsWow64Process(hProc, &wow64);
    if (wow64) { CloseHandle(hProc); return false; }

    SIZE_T bytes = (dllPath.size() + 1) * sizeof(wchar_t);
    LPVOID remote = VirtualAllocEx(hProc, nullptr, bytes, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    bool ok = false;
    if (remote && WriteProcessMemory(hProc, remote, dllPath.c_str(), bytes, nullptr))
    {
        auto loadLib = (LPTHREAD_START_ROUTINE)GetProcAddress(
            GetModuleHandleW(L"kernel32.dll"), "LoadLibraryW");
        HANDLE th = CreateRemoteThread(hProc, nullptr, 0, loadLib, remote, 0, nullptr);
        if (th)
        {
            WaitForSingleObject(th, 10000);
            CloseHandle(th);
            ok = true;
        }
    }
    if (remote)
        VirtualFreeEx(hProc, remote, 0, MEM_RELEASE);
    CloseHandle(hProc);
    return ok;
}

int WINAPI wWinMain(HINSTANCE, HINSTANCE, LPWSTR, int)
{
    // Instância única
    HANDLE mutex = CreateMutexW(nullptr, TRUE, L"Local\\ClassicPanelInjector");
    if (GetLastError() == ERROR_ALREADY_EXISTS)
        return 0;

    wchar_t exePath[MAX_PATH];
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);
    std::wstring dir = exePath;
    dir.erase(dir.find_last_of(L'\\') + 1);
    std::wstring dllName = L"classic_panel.dll";
    std::wstring dllPath = dir + dllName;

    if (GetFileAttributesW(dllPath.c_str()) == INVALID_FILE_ATTRIBUTES)
    {
        MessageBoxW(nullptr, L"classic_panel.dll nao encontrada ao lado do injetor.",
                    L"Classic Panel", MB_ICONERROR);
        return 1;
    }

    DWORD mySession = 0;
    ProcessIdToSessionId(GetCurrentProcessId(), &mySession);

    std::set<DWORD> failed; // evita tentar o mesmo PID para sempre

    for (;;)
    {
        HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (snap != INVALID_HANDLE_VALUE)
        {
            PROCESSENTRY32W pe = { sizeof(pe) };
            if (Process32FirstW(snap, &pe))
            {
                do
                {
                    if (!IsTarget(pe.szExeFile) || failed.count(pe.th32ProcessID))
                        continue;

                    DWORD session = 0;
                    if (!ProcessIdToSessionId(pe.th32ProcessID, &session) || session != mySession)
                        continue;

                    if (AlreadyInjected(pe.th32ProcessID, dllName))
                        continue;

                    if (!Inject(pe.th32ProcessID, dllPath))
                        failed.insert(pe.th32ProcessID);
                } while (Process32NextW(snap, &pe));
            }
            CloseHandle(snap);
        }
        Sleep(500);
    }

    CloseHandle(mutex);
    return 0;
}
