#include "App.h"
#include "I18n.h"
#include "resource.h"

static const wchar_t* MUTEX_NAME = L"FuWinLauncher_SingleInstance_Mutex";

App::~App() {
    RemoveTrayIcon();
    if (m_mutex) {
        ReleaseMutex(m_mutex);
        CloseHandle(m_mutex);
    }
}

bool App::Init(HINSTANCE hInstance) {
    m_hInstance = hInstance;

    // Prevent multiple instances
    m_mutex = CreateMutexW(nullptr, TRUE, MUTEX_NAME);
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        // Bring the existing window to the foreground
        HWND existing = FindWindowW(L"FuWinLauncherClass", nullptr);
        if (existing) {
            ShowWindow(existing, SW_SHOW);
            SetForegroundWindow(existing);
        }
        CloseHandle(m_mutex);
        m_mutex = nullptr;
        return false;
    }

    // Initialize i18n with OS language first
    I18n::Get().DetectFromOS();

    std::wstring configPath = GetConfigPath();

    if (!m_config.Load(configPath)) {
        // Config not found or empty - create default
        m_config.CreateDefault(configPath);
    }

    // Override language from config if specified
    if (!m_config.GetLanguage().empty()) {
        I18n::Get().SetLangFromString(m_config.GetLanguage());
    }

    // Apply skin overrides if configured
    if (!m_config.GetSkin().empty()) {
        std::wstring exeDir = configPath.substr(0, configPath.find_last_of(L"\\/") + 1);
        m_config.LoadSkin(m_config.GetSkin(), exeDir);
    }

    if (!m_mainWindow.Create(hInstance, m_config)) {
        MessageBoxW(nullptr, I18n::Get().T("err.window"),
                    L"FuWinLauncher", MB_OK | MB_ICONERROR);
        return false;
    }

    m_mainWindow.SetConfig(&m_config);
    m_mainWindow.SetApps(m_config.GetApps());

    // Register hotkey
    RegisterHotKey(m_mainWindow.GetHWND(), IDH_HOTKEY,
                   m_config.GetHotKeyModifiers() | MOD_NOREPEAT,
                   m_config.GetHotKeyVK());

    // Register tray icon
    AddTrayIcon();

    m_mainWindow.Show();
    return true;
}

int App::Run() {
    MSG msg = {};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        if (msg.message == WM_HOTKEY && msg.wParam == IDH_HOTKEY) {
            m_mainWindow.Toggle();
            continue;
        }

        // Forward Tab/Enter/Esc/arrow keys to SearchBox
        if (msg.message == WM_KEYDOWN) {
            WPARAM vk = msg.wParam;
            if (vk == VK_ESCAPE || vk == VK_RETURN ||
                vk == VK_UP || vk == VK_DOWN) {
                SendMessageW(m_mainWindow.GetHWND(), WM_KEYDOWN, vk, 0);
                continue;
            }
        }

        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    UnregisterHotKey(m_mainWindow.GetHWND(), IDH_HOTKEY);
    return static_cast<int>(msg.wParam);
}

void App::AddTrayIcon() {
    m_nid = {};
    m_nid.cbSize = sizeof(m_nid);
    m_nid.hWnd = m_mainWindow.GetHWND();
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
    m_nid.uCallbackMessage = WM_TRAYICON;
    m_nid.hIcon = LoadIconW(m_hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));
    wcscpy_s(m_nid.szTip, L"FuWinLauncher");
    Shell_NotifyIconW(NIM_ADD, &m_nid);
}

void App::RemoveTrayIcon() {
    Shell_NotifyIconW(NIM_DELETE, &m_nid);
}

std::wstring App::GetConfigPath() const {
    wchar_t exePath[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, exePath, MAX_PATH);

    // Get the directory containing the EXE
    std::wstring path(exePath);
    size_t lastSlash = path.find_last_of(L"\\/");
    if (lastSlash != std::wstring::npos) {
        path = path.substr(0, lastSlash + 1);
    }
    path += L"config.ini";
    return path;
}
