#include "SearchBox.h"
#include "resource.h"
#include <algorithm>

bool SearchBox::Create(HWND parent, int x, int y, int width, int height) {
    m_font = CreateFontW(
        24, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN,
        L"Meiryo UI"
    );

    m_hwnd = CreateWindowExW(
        0,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | ES_LEFT | ES_AUTOHSCROLL,
        x, y, width, height,
        parent,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_SEARCH_EDIT)),
        GetModuleHandleW(nullptr),
        nullptr
    );

    if (!m_hwnd) return false;

    SendMessageW(m_hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
    return true;
}

void SearchBox::SetColors(COLORREF bg, COLORREF text) {
    m_bgColor = bg;
    m_textColor = text;
    if (m_bgBrush) DeleteObject(m_bgBrush);
    m_bgBrush = CreateSolidBrush(m_bgColor);
    if (m_hwnd) InvalidateRect(m_hwnd, nullptr, TRUE);
}

void SearchBox::Clear() {
    if (m_hwnd) {
        SetWindowTextW(m_hwnd, L"");
    }
}

void SearchBox::SetFocusToEdit() {
    if (m_hwnd) {
        SetFocus(m_hwnd);
    }
}

std::wstring SearchBox::GetText() const {
    if (!m_hwnd) return L"";
    int len = GetWindowTextLengthW(m_hwnd);
    if (len == 0) return L"";
    std::wstring text(len + 1, L'\0');
    GetWindowTextW(m_hwnd, text.data(), len + 1);
    text.resize(len);
    return text;
}

std::vector<const AppEntry*> SearchBox::Filter(
    const std::vector<AppEntry>& apps,
    const std::wstring& query)
{
    std::vector<const AppEntry*> result;

    if (query.empty()) {
        for (const auto& app : apps) {
            result.push_back(&app);
        }
        return result;
    }

    // Convert query to lowercase
    std::wstring lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(),
                   lowerQuery.begin(), ::towlower);

    for (const auto& app : apps) {
        std::wstring lowerName = app.name;
        std::transform(lowerName.begin(), lowerName.end(),
                       lowerName.begin(), ::towlower);

        if (lowerName.find(lowerQuery) != std::wstring::npos) {
            result.push_back(&app);
        }
    }

    return result;
}
