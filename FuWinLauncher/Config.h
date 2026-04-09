#pragma once

#include <windows.h>
#include <string>
#include <vector>

struct AppEntry {
    std::wstring name;
    std::wstring path;
    HICON icon = nullptr;
};

struct ThemeConfig {
    std::wstring titleText = L"FuWinLauncher";
    COLORREF titleBarColor = RGB(30, 30, 40);
    COLORREF titleTextColor = RGB(255, 255, 255);
    COLORREF bgColor = RGB(30, 30, 40);
    COLORREF textColor = RGB(240, 240, 240);
    COLORREF selectColor = RGB(60, 80, 120);
    COLORREF searchBgColor = RGB(45, 45, 60);
    COLORREF searchTextColor = RGB(240, 240, 240);
    std::wstring bgImage;
    BYTE bgImageAlpha = 40;  // 0-255, default subtle
    int bgImageMode = 0;     // 0=center, 1=stretch, 2=tile
    std::wstring customIcon;
};

class Config {
public:
    bool Load(const std::wstring& iniPath);
    void CreateDefault(const std::wstring& iniPath);
    bool AppendApp(const std::wstring& name, const std::wstring& path);
    void SaveApps();
    void Save();
    bool LoadSkin(const std::wstring& skinName, const std::wstring& exeDir);
    const std::wstring& GetIniPath() const { return m_iniPath; }

    const std::vector<AppEntry>& GetApps() const { return m_apps; }
    std::vector<AppEntry>& GetApps() { return m_apps; }
    BYTE GetOpacity() const { return m_opacity; }
    UINT GetHotKeyModifiers() const { return m_hotKeyMod; }
    UINT GetHotKeyVK() const { return m_hotKeyVK; }
    int GetMaxHeight() const { return m_maxHeight; }
    bool GetTopmost() const { return m_topmost; }
    bool GetHideOnLaunch() const { return m_hideOnLaunch; }
    void SetHideOnLaunch(bool v) { m_hideOnLaunch = v; }
    bool GetShowSettingsButton() const { return m_showSettingsButton; }
    void SetShowSettingsButton(bool v) { m_showSettingsButton = v; }
    bool GetShowHelpButton() const { return m_showHelpButton; }
    void SetShowHelpButton(bool v) { m_showHelpButton = v; }
    const std::wstring& GetLanguage() const { return m_language; }
    void SetLanguage(const std::wstring& v) { m_language = v; }
    const std::wstring& GetSkin() const { return m_skin; }
    void SetSkin(const std::wstring& v) { m_skin = v; }
    const ThemeConfig& GetTheme() const { return m_theme; }
    ThemeConfig& GetTheme() { return m_theme; }
    const ThemeConfig& GetBaseTheme() const { return m_baseTheme; }
    ThemeConfig& GetBaseTheme() { return m_baseTheme; }
    void ResetThemeToBase() { m_theme = m_baseTheme; }

    void SetOpacity(BYTE v) { m_opacity = v; }
    void SetHotKey(UINT mod, UINT vk) { m_hotKeyMod = mod; m_hotKeyVK = vk; }
    void SetMaxHeight(int v) { m_maxHeight = v; }
    void SetTopmost(bool v) { m_topmost = v; }

    static std::wstring HotKeyToString(UINT mod, UINT vk);
    static COLORREF ParseColor(const std::string& str, COLORREF def);
    static std::string ColorToString(COLORREF c);

private:
    void ParseHotKey(const std::wstring& str);

    std::wstring m_iniPath;
    std::vector<AppEntry> m_apps;
    BYTE m_opacity = 200;
    UINT m_hotKeyMod = MOD_ALT;
    UINT m_hotKeyVK = VK_SPACE;
    int m_maxHeight = 600;
    bool m_topmost = true;
    bool m_hideOnLaunch = false;
    bool m_showSettingsButton = true;
    bool m_showHelpButton = true;
    std::wstring m_language;
    std::wstring m_skin;
    ThemeConfig m_theme;       // active theme (base + skin overrides)
    ThemeConfig m_baseTheme;   // user-edited theme (saved to config.ini)
};
