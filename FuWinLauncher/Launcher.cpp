#include "Launcher.h"
#include <shellapi.h>

std::wstring Launcher::ExpandPath(const std::wstring& path) {
    wchar_t expanded[MAX_PATH] = {};
    DWORD len = ExpandEnvironmentStringsW(path.c_str(), expanded, MAX_PATH);
    if (len > 0 && len <= MAX_PATH) {
        return std::wstring(expanded);
    }
    return path;
}

bool Launcher::Launch(const std::wstring& path) {
    std::wstring expandedPath = ExpandPath(path);

    // .ps1 files need to be launched via powershell.exe
    // because ShellExecuteW opens them in Notepad by default
    std::wstring ext;
    size_t dot = expandedPath.rfind(L'.');
    if (dot != std::wstring::npos) {
        ext = expandedPath.substr(dot);
        for (auto& c : ext) c = towlower(c);
    }

    HINSTANCE result;
    if (ext == L".ps1") {
        // Use the full System32 path so a powershell.exe in the current
        // directory or on PATH is never picked up instead
        wchar_t sysDir[MAX_PATH] = {};
        UINT sysLen = GetSystemDirectoryW(sysDir, MAX_PATH);
        if (sysLen == 0 || sysLen >= MAX_PATH) return false;
        std::wstring powershell = std::wstring(sysDir) +
            L"\\WindowsPowerShell\\v1.0\\powershell.exe";

        std::wstring args = L"-ExecutionPolicy Bypass -File \"" + expandedPath + L"\"";
        result = ShellExecuteW(
            nullptr, L"open", powershell.c_str(),
            args.c_str(), nullptr, SW_SHOWNORMAL
        );
    } else {
        result = ShellExecuteW(
            nullptr, L"open", expandedPath.c_str(),
            nullptr, nullptr, SW_SHOWNORMAL
        );
    }

    return reinterpret_cast<INT_PTR>(result) > 32;
}
