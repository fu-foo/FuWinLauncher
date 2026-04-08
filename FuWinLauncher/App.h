#pragma once

#include <windows.h>
#include <shellapi.h>
#include "Config.h"
#include "MainWindow.h"

// WM_TRAYICON is defined in resource.h

class App {
public:
    ~App();
    bool Init(HINSTANCE hInstance);
    int Run();

private:
    std::wstring GetConfigPath() const;
    void AddTrayIcon();
    void RemoveTrayIcon();

    HINSTANCE m_hInstance = nullptr;
    HANDLE m_mutex = nullptr;
    Config m_config;
    MainWindow m_mainWindow;
    NOTIFYICONDATAW m_nid = {};
};
