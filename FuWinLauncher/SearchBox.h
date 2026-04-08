#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "Config.h"

class SearchBox {
public:
    bool Create(HWND parent, int x, int y, int width, int height);
    HWND GetHWND() const { return m_hwnd; }
    void Clear();
    void SetFocusToEdit();
    std::wstring GetText() const;
    void SetColors(COLORREF bg, COLORREF text);
    HBRUSH GetBgBrush() const { return m_bgBrush; }
    COLORREF GetTextColor() const { return m_textColor; }

    static std::vector<const AppEntry*> Filter(
        const std::vector<AppEntry>& apps,
        const std::wstring& query);

private:
    HWND m_hwnd = nullptr;
    HFONT m_font = nullptr;
    HBRUSH m_bgBrush = nullptr;
    COLORREF m_bgColor = RGB(45, 45, 60);
    COLORREF m_textColor = RGB(240, 240, 240);
};
