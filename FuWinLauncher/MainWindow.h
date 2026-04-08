#pragma once

#include <windows.h>
#include <vector>
#include <gdiplus.h>
#include "Config.h"
#include "SearchBox.h"
#include "Launcher.h"

class MainWindow {
public:
    ~MainWindow();
    bool Create(HINSTANCE hInstance, const Config& config);
    void Show();
    void Hide();
    void Toggle();
    bool IsShown() const { return m_visible; }
    HWND GetHWND() const { return m_hwnd; }

    void SetApps(std::vector<AppEntry>& apps);
    void SetConfig(Config* config) { m_config = config; }
    void UpdateFilter();

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam);

    void OnPaint();
    void OnKeyDown(WPARAM vk);
    void OnCommand(WPARAM wParam, LPARAM lParam);
    void OnDropFiles(HDROP hDrop);
    void OnRightClick(int x, int y);
    void DeleteApp(int index);
    void EditApp(int index);
    void AddNewApp();
    bool ShowEditDialog(const wchar_t* title, std::wstring& name, std::wstring& path);
    void ReloadFromConfig();
    void LaunchSelected();
    void ToggleTopmost();
    void UpdatePinButton();
    int HitTest(int y) const;
    bool IsInScrollbar(int x) const;
    void EnsureVisible(int index);
    void ResizeToFit();
    int CalcClientHeight(int appCount) const;
    void LoadIcons(std::vector<AppEntry>& apps);
    int GetMaxScroll() const;
    void SaveAppOrder();
    void LoadBgImage();
    void ApplyTheme(const ThemeConfig& theme);

    HWND m_hwnd = nullptr;
    HWND m_pinButton = nullptr;
    HWND m_settingsButton = nullptr;
    HWND m_helpButton = nullptr;
    HINSTANCE m_hInstance = nullptr;
    bool m_visible = true;
    bool m_topmost = true;
    DWORD m_exStyle = 0;
    int m_maxHeight = 600;
    SearchBox m_searchBox;
    HFONT m_fontName = nullptr;
    ThemeConfig m_theme;
    Gdiplus::Bitmap* m_bgImage = nullptr;
    ULONG_PTR m_gdiplusToken = 0;

    Config* m_config = nullptr;
    const std::vector<AppEntry>* m_allApps = nullptr;
    std::vector<const AppEntry*> m_filtered;
    int m_selectedIndex = 0;
    int m_scrollOffset = 0;
    int m_windowWidth = 500;

    bool m_draggingScrollbar = false;
    int m_dragStartY = 0;
    int m_dragStartScroll = 0;

    bool m_draggingItem = false;
    int m_dragItemIndex = -1;
    int m_dragInsertIndex = -1;
    POINT m_dragStartPt = {};
    static constexpr int DRAG_THRESHOLD = 5;

    static constexpr int SEARCH_HEIGHT = 40;
    static constexpr int SEARCH_MARGIN = 20;
    static constexpr int ICON_SIZE = 32;
    static constexpr int ITEM_HEIGHT = 40;
    static constexpr int ITEM_PADDING = 8;
    static constexpr int LIST_TOP_MARGIN = 80;
    static constexpr int BOTTOM_MARGIN = 10;
    static constexpr int DEFAULT_APP_COUNT = 5;
    static constexpr int SCROLLBAR_WIDTH = 14;
};
