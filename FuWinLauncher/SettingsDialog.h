#pragma once

#include <windows.h>
#include "Config.h"

class SettingsDialog {
public:
    static bool Show(HWND parent, HINSTANCE hInstance, Config& config);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    void CreateControls(HWND hwnd);
    void OnOpacityChanged();
    void OnBgAlphaChanged();
    void OnOK(HWND hwnd);
    void UpdateThemeControlsEnabled(HWND hwnd);
    void PickColor(HWND hwnd, HWND editCtrl);
    void BrowseFile(HWND hwnd, HWND editCtrl, const wchar_t* filter);

    HINSTANCE m_hInstance = nullptr;
    Config* m_config = nullptr;
    bool m_saved = false;
    bool m_closed = false;

    // Settings controls
    HWND m_opacitySlider = nullptr;
    HWND m_opacityLabel = nullptr;
    HWND m_maxHeightEdit = nullptr;
    HWND m_langCombo = nullptr;
    HWND m_hotkeyCombo = nullptr;
    HWND m_skinCombo = nullptr;
    HWND m_topmostCheck = nullptr;
    HWND m_showSettingsCheck = nullptr;
    HWND m_showHelpCheck = nullptr;
    HWND m_hideOnLaunchCheck = nullptr;

    // Theme controls
    HWND m_titleTextEdit = nullptr;
    HWND m_titleBarColorEdit = nullptr;
    HWND m_titleTextColorEdit = nullptr;
    HWND m_bgColorEdit = nullptr;
    HWND m_textColorEdit = nullptr;
    HWND m_selectColorEdit = nullptr;
    HWND m_searchBgColorEdit = nullptr;
    HWND m_searchTextColorEdit = nullptr;
    HWND m_bgImageEdit = nullptr;
    HWND m_bgModeCombo = nullptr;
    HWND m_bgAlphaSlider = nullptr;
    HWND m_bgAlphaLabel = nullptr;
    HWND m_customIconEdit = nullptr;

    HFONT m_font = nullptr;
};
