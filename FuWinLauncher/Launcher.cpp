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

    HINSTANCE result = ShellExecuteW(
        nullptr, L"open", expandedPath.c_str(),
        nullptr, nullptr, SW_SHOWNORMAL
    );

    return reinterpret_cast<INT_PTR>(result) > 32;
}
