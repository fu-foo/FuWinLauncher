#pragma once

#include <windows.h>
#include <string>

class Launcher {
public:
    static bool Launch(const std::wstring& path);
    static std::wstring ExpandPath(const std::wstring& path);
};
