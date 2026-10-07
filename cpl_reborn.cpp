// Control Panel Reborn - versão standalone (DLL)
// Port do mod Windhawk "cpl-reborn" de aubymori.
// Hook: MinHook. Resolução de símbolo privado: dbghelp + Microsoft Symbol Server.

#include <windows.h>
#include <commctrl.h>
#include <psapi.h>
#include <shlobj.h>
#include <dbghelp.h>
#include <cstdio>
#include <cstdarg>
#include <cstring>
#include <cwchar>
#include "MinHook.h"

// ---------------------------------------------------------------- log
static void Log(const wchar_t *fmt, ...)
{
    wchar_t buf[512];
    va_list ap;
    va_start(ap, fmt);
    _vsnwprintf_s(buf, _countof(buf), _TRUNCATE, fmt, ap);
    va_end(ap);
    OutputDebugStringW(L"[CplReborn] ");
    OutputDebugStringW(buf);
    OutputDebugStringW(L"\n");
}

// ------------------------------------------------------ ordem dos applets
/**
 * Arrays com menos de 20 itens precisam ser preenchidos com NULL.
 */
static LPCWSTR g_szAppletOrder[][20] = {
    /* System and Security */
    {
        L"::{BB64F8A7-BEE7-4E1A-AB8D-7D8273F7FDB6}", // Action Center
        L"::{4026492F-2F69-46B8-B9BF-5654FC07E423}", // Windows Firewall
        L"::{BB06C0E4-D293-4f75-8A90-CB05B6477EEE}", // System
        L"::{36eef7db-88ad-4e81-ad49-0e313f0c35f8}", // Windows Update
        L"::{025A5937-A6BE-4686-A844-36FE4BEC8B6D}", // Power Options
        L"::{87D66A43-7B11-4A28-9811-C86EE395ACF7}", // Indexing Options
        L"::{F6B6E965-E9B2-444B-9286-10C9152EDBC5}", // File History
        L"::{B98A2BEA-7D42-4558-8BD1-832F41BAC6FD}", // Backup and Restore
        L"::{D9EF8727-CAC2-4e60-809E-86F80A666C91}", // BitLocker Drive Encryption
        L"::{F942C606-0914-47AB-BE56-1321B8035096}", // Storage Spaces
        L"::{ECDB0924-4208-451E-8EE0-373C0956DE16}", // Work Folders
        L"::{BE122A0E-4503-11DA-8BDE-F66BAD1E3F3A}", // Add features to Windows
        L"::{78F3955E-3B90-4184-BD14-5397C15F1EFC}", // WEI
        L"::{74246bfc-4c96-11d0-abef-0020af6b0b7a}", // Device Manager
        L"::{D20EA4E1-3957-11d2-A40B-0C5020524153}", // Administrative Tools
        NULL, NULL, NULL, NULL, NULL,
    },
    /* Network and Internet */
    {
        L"::{8E908FC9-BECC-40f6-915B-F4CA0E70D03D}", // Network and Sharing Center
        L"::{67CA7650-96E6-4FDD-BB43-A8E774F73A57}", // HomeGroup
        L"::{A3DD4F92-658A-410F-84FD-6FBBBEF2FFFE}", // Internet Options
        L"::{4026492F-2F69-46B8-B9BF-5654FC07E423}", // Windows Firewall
        L"::{9C73F5E5-7AE7-4E32-A8E8-8D23B85255BF}", // Sync Center
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    /* Hardware and Sound */
    {
        L"::{2227A280-3AEA-1069-A2DE-08002B30309D}", // Printers
        L"::{A8A91A66-3A7D-4424-8D24-04E180695C7A}", // Devices and Printers
        L"::{9C60DE1E-E5FC-40f4-A487-460851A8D915}", // AutoPlay
        L"::{F2DDFC82-8F12-4CDD-B7DC-D4FE1425AA4D}", // Sound
        L"::{6C8EEC18-8D75-41B2-A177-8831D59D2D50}", // Mouse
        L"::{025A5937-A6BE-4686-A844-36FE4BEC8B6D}", // Power Options
        L"::{ED834ED6-4B5A-4bfe-8F11-A626DCB6A921}", // Personalization
        L"::{725BE8F7-668E-4C7B-8F90-46BDB0936430}", // Keyboard
        L"::{74246bfc-4c96-11d0-abef-0020af6b0b7a}", // Device Manager
        L"::{40419485-C444-4567-851A-2DD7BFA1684D}", // Phone and Modem
        L"::{259EF4B1-E6C9-4176-B574-481532C9BCE8}", // Game Controllers
        L"::{C555438B-3C23-4769-A71F-B6D3D9B6053A}", // Display
        L"::{5ea4f148-308c-46d7-98a9-49041b1dd468}", // Windows Mobility Center
        L"::{F82DF8F7-8B9F-442E-A48C-818EA735FF9B}", // Pen and Touch
        L"::{E9950154-C418-419e-A90A-20C5287AE24B}", // Location Settings
        L"::{0142e4d0-fb7a-11dc-ba4a-000ffe7ab428}", // Biometric Devices
        NULL, NULL, NULL, NULL,
    },
    /* Programs */
    {
        L"::{7B81BE6A-CE2B-4676-A29E-EB907A5126C5}", // Programs and Features
        L"::{17cd9488-1228-4b2f-88ce-4298e93e0966}", // Default Programs
        L"::{37efd44d-ef8d-41b1-940d-96973a50e9e0}", // Desktop Gadgets
        L"::{ED7BA470-8E54-465E-825C-99712043E01C}", // GodMode
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    /* User Accounts and Family Safety */
    {
        L"::{60632754-c523-4b62-b45c-4172da012619}", // User Accounts
        L"::{96AE8D84-A250-4520-95A5-A47A7E3C548B}", // Family Safety
        L"::{78CB147A-98EA-4AA6-B0DF-C8681F69341C}", // Windows CardSpace
        L"::{1206F5F1-0569-412C-8FEC-3204630DFB70}", // Credential Manager
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    /* Appearance and Personalization */
    {
        L"::{ED834ED6-4B5A-4bfe-8F11-A626DCB6A921}", // Personalization
        L"::{C555438B-3C23-4769-A71F-B6D3D9B6053A}", // Display
        L"::{37efd44d-ef8d-41b1-940d-96973a50e9e0}", // Desktop Gadgets
        L"::{0DF44EAA-FF21-4412-828E-260A8728E7F1}", // Taskbar and Navigation
        L"::{D555645E-D4F8-4c29-A827-D93C859C4F2A}", // Ease of Access Center
        L"::{6DFD7C5C-2451-11d3-A299-00C04F8EF6AF}", // Folder Options
        L"::{BD84B380-8CA2-1069-AB1D-08000948F534}", // Fonts
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    /* Clock, Language, and Region */
    {
        L"::{E2E7934B-DCE5-43C4-9576-7FE4F75E7480}", // Date and Time
        L"::{BF782CC9-5A52-4A17-806C-2A894FFEEAC5}", // Language
        L"::{62d8ed13-c9d0-4ce8-a914-47dd628fb1b0}", // Region
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
    /* Ease of Access */
    {
        L"::{D555645E-D4F8-4c29-A827-D93C859C4F2A}", // Ease of Access Center
        L"::{58E3C745-D971-4081-9034-86E34B30836A}", // Speech Recognition
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    },
};

#define CControlPanelAppletList_HDPA(pThis)     *((HDPA *)(pThis) + 2)
#define CControlPanelAppletList_Category(pThis) *((DWORD *)(pThis) + 32)

static int MapCategory(int category)
{
    switch (category)
    {
        case 5: return 0; // System and Security
        case 3: return 1; // Network and Internet
        case 2: return 2; // Hardware and Sound
        case 8: return 3; // Programs
        case 9: return 4; // User Accounts and Family Safety
        case 1: return 5; // Appearance and Personalization
        case 6: return 6; // Clock, Language, and Region
        case 7: return 7; // Ease of Access
        default: return -1;
    }
}

static int FindApplet(LPCWSTR lpszApplet, int category)
{
    for (UINT i = 0; i < _countof(g_szAppletOrder[category]); i++)
    {
        if (NULL == g_szAppletOrder[category][i])
            break;
        if (0 == _wcsicmp(g_szAppletOrder[category][i], lpszApplet))
            return (int)i;
    }
    return -1;
}

// ------------------------------------------------------------- o hook
typedef int (*SortApplets_t)(void *, void *, LPARAM);
static SortApplets_t g_origSort = nullptr;

static int SortApplets_hook(void *p1, void *p2, LPARAM lParam)
{
    HDPA hDpa = CControlPanelAppletList_HDPA(lParam);
    LPVOID pThing1 = DPA_GetPtr(hDpa, *(int *)p1);
    LPVOID pThing2 = DPA_GetPtr(hDpa, *(int *)p2);
    int category = MapCategory((int)CControlPanelAppletList_Category(lParam));
    if (category >= 0 && hDpa && pThing1 && pThing2)
    {
        LPCWSTR pszApplet1 = (LPCWSTR)((char *)pThing1 + 520);
        LPCWSTR pszApplet2 = (LPCWSTR)((char *)pThing2 + 520);

        int iApplet1 = FindApplet(pszApplet1, category);
        if (iApplet1 < 0)
            return 1;

        int iApplet2 = FindApplet(pszApplet2, category);
        if (iApplet2 >= 0 && iApplet1 > iApplet2)
            return 1;
        return -1;
    }
    return 0;
}

// ------------------------------------------- strings a "apagar" no shell32
static LPCWSTR g_szAppletsToUnhide[] = {
    L"::{BF782CC9-5A52-4A17-806C-2A894FFEEAC5}", // Language Settings
    L"::{ED834ED6-4B5A-4BFE-8F11-A626DCB6A921}", // Personalization
    L"::{D9EF8727-CAC2-4e60-809E-86F80A666C91}", // BitLocker Drive Encryption
    L"::{D17D1D6D-CC3F-4815-8FE3-607E7D5D10B3}", // Text to Speech
    L"::{E95A4861-D57A-4BE1-AD0F-35267E261739}", // Windows SlideShow
    L"::{96AE8D84-A250-4520-95A5-A47A7E3C548B}", // Family Safety
    L"::{BB06C0E4-D293-4f75-8A90-CB05B6477EEE}", // System
};

static LPCWSTR g_szCanonicalNames[] = {
    L"Microsoft.Display",
    L"Microsoft.Personalization",
    L"Microsoft.Language",
    L"Microsoft.LocationAndOtherSensors",
    L"Microsoft.LocationSettings",
    L"Microsoft.WindowsUpdate",
    L"Microsoft.Troubleshooting",
    L"Microsoft.DevicesAndPrinters",
    L"Microsoft.RegionalAndLanguageOptions",
    L"Microsoft.System",
    L"::{BB06C0E4-D293-4f75-8A90-CB05B6477EEE}",
    L"bthprops.cpl",
};

static const BYTE *FindBytes(const BYTE *hay, size_t hayLen, const BYTE *needle, size_t needleLen)
{
    if (needleLen == 0 || hayLen < needleLen)
        return nullptr;
    for (size_t i = 0; i + needleLen <= hayLen; i++)
    {
        if (hay[i] == needle[0] && memcmp(hay + i, needle, needleLen) == 0)
            return hay + i;
    }
    return nullptr;
}

static void ZeroStringInModule(HMODULE hModule, LPCWSTR lpSearch)
{
    if (!lpSearch)
        return;

    MODULEINFO info = {};
    if (!GetModuleInformation(GetCurrentProcess(), hModule, &info, sizeof(info)))
        return;

    BYTE *base = (BYTE *)info.lpBaseOfDll;
    BYTE *end = base + info.SizeOfImage;
    size_t patternLen = wcslen(lpSearch) * sizeof(wchar_t);

    // Percorre as regiões de memória do módulo (só as legíveis)
    BYTE *p = base;
    while (p < end)
    {
        MEMORY_BASIC_INFORMATION mbi;
        if (!VirtualQuery(p, &mbi, sizeof(mbi)))
            break;

        BYTE *regionEnd = (BYTE *)mbi.BaseAddress + mbi.RegionSize;
        if (regionEnd > end)
            regionEnd = end;

        bool readable = mbi.State == MEM_COMMIT
            && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD));

        if (readable)
        {
            const BYTE *hit = FindBytes(p, (size_t)(regionEnd - p),
                                        (const BYTE *)lpSearch, patternLen);
            if (hit)
            {
                Log(L"Found \"%s\"", lpSearch);
                DWORD oldProt = 0;
                if (VirtualProtect((LPVOID)hit, patternLen, PAGE_READWRITE, &oldProt))
                {
                    ZeroMemory((void *)hit, patternLen);
                    DWORD tmp;
                    VirtualProtect((LPVOID)hit, patternLen, oldProt, &tmp);
                }
                return;
            }
        }
        p = regionEnd;
    }
    Log(L"Did not find \"%s\"", lpSearch);
}

// --------------------------------- resolução do símbolo privado (dbghelp)
static HMODULE g_hSelf = nullptr;

static void GetOwnDir(wchar_t *out, size_t cch)
{
    GetModuleFileNameW(g_hSelf, out, (DWORD)cch);
    wchar_t *slash = wcsrchr(out, L'\\');
    if (slash) *(slash + 1) = 0;
}

struct FindCtx { PVOID addr; };

static BOOL CALLBACK EnumSymCb(PSYMBOL_INFO si, ULONG, PVOID ctx)
{
    // Funciona tanto com nome decorado quanto com nome limpo (SYMOPT_UNDNAME)
    if (strstr(si->Name, "CControlPanelAppletList") &&
        strstr(si->Name, "s_SortAppletsInCategory"))
    {
        ((FindCtx *)ctx)->addr = (PVOID)si->Address;
        return FALSE; // para a enumeração
    }
    return TRUE;
}

static PVOID ResolveSortFunction(HMODULE hShell32)
{
    // Carrega o dbghelp.dll que está ao lado da DLL (junto com symsrv.dll),
    // pois o dbghelp do System32 não sabe baixar símbolos da Microsoft.
    wchar_t dir[MAX_PATH];
    GetOwnDir(dir, MAX_PATH);
    wchar_t dbgPath[MAX_PATH];
    swprintf_s(dbgPath, L"%sdbghelp.dll", dir);
    if (!LoadLibraryW(dbgPath))
        Log(L"dbghelp.dll local nao encontrado; usando o do sistema (pode falhar)");

    wchar_t localApp[MAX_PATH] = {};
    SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, localApp);
    wchar_t cache[MAX_PATH];
    swprintf_s(cache, L"%s\\CplReborn\\symbols", localApp);
    SHCreateDirectoryExW(nullptr, cache, nullptr);

    char symPath[2 * MAX_PATH];
    sprintf_s(symPath, "srv*%ls*https://msdl.microsoft.com/download/symbols", cache);

    HANDLE hProc = GetCurrentProcess();
    SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_FAIL_CRITICAL_ERRORS);
    if (!SymInitialize(hProc, symPath, FALSE))
    {
        Log(L"SymInitialize falhou: %lu", GetLastError());
        return nullptr;
    }

    MODULEINFO mi = {};
    GetModuleInformation(hProc, hShell32, &mi, sizeof(mi));
    char shellPath[MAX_PATH];
    GetModuleFileNameA(hShell32, shellPath, MAX_PATH);

    DWORD64 loaded = SymLoadModuleEx(hProc, nullptr, shellPath, nullptr,
                                     (DWORD64)mi.lpBaseOfDll, mi.SizeOfImage,
                                     nullptr, 0);
    if (!loaded && GetLastError() != ERROR_SUCCESS)
    {
        Log(L"SymLoadModuleEx falhou: %lu", GetLastError());
        SymCleanup(hProc);
        return nullptr;
    }

    FindCtx ctx = { nullptr };
    SymEnumSymbols(hProc, (DWORD64)mi.lpBaseOfDll, "*s_SortAppletsInCategory*",
                   EnumSymCb, &ctx);
    SymCleanup(hProc);

    if (!ctx.addr)
        Log(L"Simbolo s_SortAppletsInCategory nao encontrado (PDB indisponivel?)");
    return ctx.addr;
}

// --------------------------------------------------------------- init
static DWORD WINAPI InitThread(LPVOID)
{
    HMODULE hShell32 = LoadLibraryW(L"shell32.dll");
    if (!hShell32)
    {
        Log(L"Falha ao carregar shell32.dll");
        return 1;
    }

    for (size_t i = 0; i < _countof(g_szAppletsToUnhide); i++)
        ZeroStringInModule(hShell32, g_szAppletsToUnhide[i]);

    for (size_t i = 0; i < _countof(g_szCanonicalNames); i++)
        ZeroStringInModule(hShell32, g_szCanonicalNames[i]);

    PVOID target = ResolveSortFunction(hShell32);
    if (!target)
        return 2;

    if (MH_Initialize() != MH_OK)
    {
        Log(L"MH_Initialize falhou");
        return 3;
    }
    if (MH_CreateHook(target, (LPVOID)SortApplets_hook, (LPVOID *)&g_origSort) != MH_OK ||
        MH_EnableHook(target) != MH_OK)
    {
        Log(L"Falha ao criar/ativar o hook");
        return 4;
    }

    Log(L"Hook ativo em %p", target);
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
        case DLL_PROCESS_ATTACH:
            g_hSelf = hInst;
            DisableThreadLibraryCalls(hInst);
            // Não fazer trabalho pesado dentro do DllMain (loader lock)
            if (HANDLE h = CreateThread(nullptr, 0, InitThread, nullptr, 0, nullptr))
                CloseHandle(h);
            break;
        case DLL_PROCESS_DETACH:
            if (!reserved)
            {
                MH_DisableHook(MH_ALL_HOOKS);
                MH_Uninitialize();
            }
            break;
    }
    return TRUE;
}
