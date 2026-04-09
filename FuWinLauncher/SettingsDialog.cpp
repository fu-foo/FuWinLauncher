#include "SettingsDialog.h"
#include "I18n.h"
#include "resource.h"
#include <commctrl.h>
#include <commdlg.h>
#pragma comment(lib, "comctl32.lib")
#include <algorithm>

static const wchar_t* SETTINGS_CLASS = L"FuWinLauncherSettingsClass";

static constexpr int DLG_W = 420;
static constexpr int DLG_H = 740;
static constexpr int MARGIN = 12;
static constexpr int CTRL_H = 24;
static constexpr int BTN_W = 60;
static constexpr int BTN_H = 28;
static constexpr int ROW_GAP = 6;
static constexpr int LABEL_W = 110;
static constexpr int SMALL_BTN = 28;

static std::wstring ColorToWStr(COLORREF c) {
    wchar_t buf[8];
    wsprintfW(buf, L"#%02X%02X%02X", GetRValue(c), GetGValue(c), GetBValue(c));
    return buf;
}

static COLORREF WStrToColor(const wchar_t* s, COLORREF def) {
    std::string str;
    for (const wchar_t* p = s; *p; ++p) str += static_cast<char>(*p);
    return Config::ParseColor(str, def);
}

bool SettingsDialog::Show(HWND parent, HINSTANCE hInstance, Config& config) {
    INITCOMMONCONTROLSEX icc = {};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_HOTKEY_CLASS | ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);

    SettingsDialog dlg;
    dlg.m_hInstance = hInstance;
    dlg.m_config = &config;

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = SETTINGS_CLASS;
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));
    RegisterClassExW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    RECT wr = { 0, 0, DLG_W, DLG_H };
    AdjustWindowRectEx(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE, 0);
    int ww = wr.right - wr.left;
    int wh = wr.bottom - wr.top;

    HWND hwnd = CreateWindowExW(
        0, SETTINGS_CLASS, I18n::Get().T("settings.title"),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        (screenW - ww) / 2, (screenH - wh) / 2, ww, wh,
        parent, nullptr, hInstance, &dlg
    );

    if (!hwnd) return false;

    EnableWindow(parent, FALSE);
    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    MSG msg;
    while (!dlg.m_closed && GetMessageW(&msg, nullptr, 0, 0)) {
        if (IsDialogMessageW(hwnd, &msg)) continue;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    EnableWindow(parent, TRUE);
    SetForegroundWindow(parent);

    if (dlg.m_font) DeleteObject(dlg.m_font);
    UnregisterClassW(SETTINGS_CLASS, hInstance);

    return dlg.m_saved;
}

LRESULT CALLBACK SettingsDialog::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SettingsDialog* self = nullptr;
    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<SettingsDialog*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
    } else {
        self = reinterpret_cast<SettingsDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }
    if (self) return self->HandleMessage(hwnd, msg, wParam, lParam);
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT SettingsDialog::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        CreateControls(hwnd);
        return 0;

    case WM_COMMAND: {
        WORD id = LOWORD(wParam);
        switch (id) {
        case IDOK:     OnOK(hwnd); break;
        case IDCANCEL: DestroyWindow(hwnd); break;
        case IDC_SET_TITLEBARCOLOR_BTN:  PickColor(hwnd, m_titleBarColorEdit); break;
        case IDC_SET_TITLETEXTCOLOR_BTN: PickColor(hwnd, m_titleTextColorEdit); break;
        case IDC_SET_BGCOLOR_BTN:        PickColor(hwnd, m_bgColorEdit); break;
        case IDC_SET_TEXTCOLOR_BTN:      PickColor(hwnd, m_textColorEdit); break;
        case IDC_SET_SELECTCOLOR_BTN:    PickColor(hwnd, m_selectColorEdit); break;
        case IDC_SET_SEARCHBGCOLOR_BTN:  PickColor(hwnd, m_searchBgColorEdit); break;
        case IDC_SET_SEARCHTEXTCOLOR_BTN:PickColor(hwnd, m_searchTextColorEdit); break;
        case IDC_SET_BGIMAGE_BTN:
            BrowseFile(hwnd, m_bgImageEdit,
                L"Images (*.png;*.jpg;*.bmp)\0*.png;*.jpg;*.jpeg;*.bmp\0All Files (*.*)\0*.*\0");
            break;
        case IDC_SET_CUSTOMICON_BTN:
            BrowseFile(hwnd, m_customIconEdit,
                L"Icons (*.ico)\0*.ico\0All Files (*.*)\0*.*\0");
            break;
        }
        return 0;
    }

    case WM_HSCROLL:
        if (reinterpret_cast<HWND>(lParam) == m_opacitySlider)
            OnOpacityChanged();
        if (reinterpret_cast<HWND>(lParam) == m_bgAlphaSlider)
            OnBgAlphaChanged();
        return 0;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        m_closed = true;
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void SettingsDialog::CreateControls(HWND hwnd) {
    m_font = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Meiryo UI");

    auto makeLabel = [&](const wchar_t* text, int x, int y, int w, int h) {
        HWND h_ = CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE,
            x, y, w, h, hwnd, nullptr, m_hInstance, nullptr);
        SendMessageW(h_, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
        return h_;
    };

    auto makeEdit = [&](const wchar_t* text, int x, int y, int w) {
        HWND h_ = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", text,
            WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL | WS_TABSTOP,
            x, y, w, CTRL_H, hwnd, nullptr, m_hInstance, nullptr);
        SendMessageW(h_, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
        return h_;
    };

    auto makeButton = [&](const wchar_t* text, int id, int x, int y, int w, int h) {
        HWND h_ = CreateWindowExW(0, L"BUTTON", text,
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            x, y, w, h, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
            m_hInstance, nullptr);
        SendMessageW(h_, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
        return h_;
    };

    int y = MARGIN;
    int ctrlX = MARGIN + LABEL_W;
    int editW = DLG_W - ctrlX - MARGIN;

    // === General Settings ===
    // Language
    makeLabel(I18n::Get().T("settings.language"), MARGIN, y + 2, LABEL_W, CTRL_H);
    m_langCombo = CreateWindowExW(0, L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
        ctrlX, y, 150, CTRL_H * 4, hwnd, nullptr, m_hInstance, nullptr);
    SendMessageW(m_langCombo, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
    SendMessageW(m_langCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"\x65E5\x672C\x8A9E"));  // 日本語
    SendMessageW(m_langCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"English"));
    {
        const auto& lang = m_config->GetLanguage();
        int sel = (I18n::Get().GetLang() == Lang::JA) ? 0 : 1;
        if (lang == L"ja") sel = 0;
        else if (lang == L"en") sel = 1;
        SendMessageW(m_langCombo, CB_SETCURSEL, sel, 0);
    }
    y += CTRL_H + ROW_GAP;

    // Hotkey
    makeLabel(I18n::Get().T("settings.hotkey"), MARGIN, y + 2, LABEL_W, CTRL_H);
    m_hotkeyCombo = CreateWindowExW(0, L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
        ctrlX, y, 150, CTRL_H * 4, hwnd, nullptr, m_hInstance, nullptr);
    SendMessageW(m_hotkeyCombo, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
    SendMessageW(m_hotkeyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Alt+Space"));
    SendMessageW(m_hotkeyCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Ctrl+Space"));
    {
        UINT mod = m_config->GetHotKeyModifiers();
        int sel = (mod & MOD_CONTROL) ? 1 : 0;
        SendMessageW(m_hotkeyCombo, CB_SETCURSEL, sel, 0);
    }
    y += CTRL_H + ROW_GAP;

    // Opacity
    makeLabel(I18n::Get().T("settings.opacity"), MARGIN, y + 2, LABEL_W, CTRL_H);
    m_opacitySlider = CreateWindowExW(0, TRACKBAR_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | TBS_HORZ | WS_TABSTOP,
        ctrlX, y, editW - 40, CTRL_H, hwnd,
        reinterpret_cast<HMENU>(IDC_SET_OPACITY), m_hInstance, nullptr);
    SendMessageW(m_opacitySlider, TBM_SETRANGE, TRUE, MAKELPARAM(50, 255));
    SendMessageW(m_opacitySlider, TBM_SETPOS, TRUE, m_config->GetOpacity());
    m_opacityLabel = makeLabel(L"", ctrlX + editW - 35, y + 2, 35, CTRL_H);
    OnOpacityChanged();
    y += CTRL_H + ROW_GAP;

    // MaxHeight
    makeLabel(I18n::Get().T("settings.maxheight"), MARGIN, y + 2, LABEL_W, CTRL_H);
    m_maxHeightEdit = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", nullptr,
        WS_CHILD | WS_VISIBLE | ES_NUMBER | WS_TABSTOP,
        ctrlX, y, 80, CTRL_H, hwnd,
        reinterpret_cast<HMENU>(IDC_SET_MAXHEIGHT), m_hInstance, nullptr);
    SendMessageW(m_maxHeightEdit, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
    { wchar_t buf[16]; wsprintfW(buf, L"%d", m_config->GetMaxHeight()); SetWindowTextW(m_maxHeightEdit, buf); }
    makeLabel(L"px", ctrlX + 85, y + 2, 30, CTRL_H);
    y += CTRL_H + ROW_GAP;

    // Checkboxes
    auto makeCheck = [&](const wchar_t* text, bool checked, int& yy) {
        HWND h_ = CreateWindowExW(0, L"BUTTON", text,
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX | WS_TABSTOP,
            MARGIN, yy, DLG_W - MARGIN * 2, CTRL_H, hwnd, nullptr, m_hInstance, nullptr);
        SendMessageW(h_, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
        SendMessageW(h_, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
        yy += CTRL_H + 2;
        return h_;
    };

    m_topmostCheck = makeCheck(I18n::Get().T("settings.topmost"), m_config->GetTopmost(), y);
    m_showSettingsCheck = makeCheck(I18n::Get().T("settings.showsettingsbtn"), m_config->GetShowSettingsButton(), y);
    m_showHelpCheck = makeCheck(I18n::Get().T("settings.showhelpbtn"), m_config->GetShowHelpButton(), y);
    m_hideOnLaunchCheck = makeCheck(I18n::Get().T("settings.hideonlaunch"), m_config->GetHideOnLaunch(), y);
    y += 6;

    // === Theme section ===
    // Separator + header
    CreateWindowExW(0, L"STATIC", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        MARGIN, y, DLG_W - MARGIN * 2, 2, hwnd, nullptr, m_hInstance, nullptr);
    y += 6;
    makeLabel(I18n::Get().T("settings.theme"), MARGIN, y, 200, CTRL_H);
    y += CTRL_H + 2;

    int colorEditW = editW - SMALL_BTN - 4;

    // Color row helper
    auto makeColorRow = [&](const char* labelKey, COLORREF color, int btnId, int& yy) -> HWND {
        makeLabel(I18n::Get().T(labelKey), MARGIN, yy + 2, LABEL_W, CTRL_H);
        HWND edit = makeEdit(ColorToWStr(color).c_str(), ctrlX, yy, colorEditW);
        makeButton(L"...", btnId, ctrlX + colorEditW + 4, yy, SMALL_BTN, CTRL_H);
        yy += CTRL_H + ROW_GAP;
        return edit;
    };

    const auto& theme = m_config->GetTheme();

    // TitleText
    makeLabel(I18n::Get().T("settings.titletext"), MARGIN, y + 2, LABEL_W, CTRL_H);
    m_titleTextEdit = makeEdit(theme.titleText.c_str(), ctrlX, y, editW);
    y += CTRL_H + ROW_GAP;

    m_titleBarColorEdit  = makeColorRow("settings.titlebarcolor",  theme.titleBarColor,  IDC_SET_TITLEBARCOLOR_BTN, y);
    m_titleTextColorEdit = makeColorRow("settings.titletextcolor", theme.titleTextColor, IDC_SET_TITLETEXTCOLOR_BTN, y);
    m_bgColorEdit        = makeColorRow("settings.bgcolor",        theme.bgColor,        IDC_SET_BGCOLOR_BTN, y);
    m_textColorEdit      = makeColorRow("settings.textcolor",      theme.textColor,      IDC_SET_TEXTCOLOR_BTN, y);
    m_selectColorEdit    = makeColorRow("settings.selectcolor",    theme.selectColor,    IDC_SET_SELECTCOLOR_BTN, y);
    m_searchBgColorEdit  = makeColorRow("settings.searchbg",      theme.searchBgColor,  IDC_SET_SEARCHBGCOLOR_BTN, y);
    m_searchTextColorEdit= makeColorRow("settings.searchtext",    theme.searchTextColor, IDC_SET_SEARCHTEXTCOLOR_BTN, y);

    // BgImage
    int browseW = 30;
    int imgEditW = editW - browseW - 4;
    makeLabel(I18n::Get().T("settings.bgimage"), MARGIN, y + 2, LABEL_W, CTRL_H);
    m_bgImageEdit = makeEdit(theme.bgImage.c_str(), ctrlX, y, imgEditW);
    makeButton(L"...", IDC_SET_BGIMAGE_BTN, ctrlX + imgEditW + 4, y, browseW, CTRL_H);
    y += CTRL_H + ROW_GAP;

    // BgImageMode
    makeLabel(I18n::Get().T("settings.bgimagemode"), MARGIN, y + 2, LABEL_W, CTRL_H);
    m_bgModeCombo = CreateWindowExW(0, L"COMBOBOX", nullptr,
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_TABSTOP,
        ctrlX, y, 150, CTRL_H * 5, hwnd, nullptr, m_hInstance, nullptr);
    SendMessageW(m_bgModeCombo, WM_SETFONT, reinterpret_cast<WPARAM>(m_font), TRUE);
    SendMessageW(m_bgModeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(I18n::Get().T("settings.mode.center")));
    SendMessageW(m_bgModeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(I18n::Get().T("settings.mode.stretch")));
    SendMessageW(m_bgModeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(I18n::Get().T("settings.mode.tile")));
    SendMessageW(m_bgModeCombo, CB_SETCURSEL, std::clamp(theme.bgImageMode, 0, 2), 0);
    y += CTRL_H + ROW_GAP;

    // BgImageAlpha
    makeLabel(I18n::Get().T("settings.bgimagealpha"), MARGIN, y + 2, LABEL_W, CTRL_H);
    m_bgAlphaSlider = CreateWindowExW(0, TRACKBAR_CLASSW, nullptr,
        WS_CHILD | WS_VISIBLE | TBS_HORZ | WS_TABSTOP,
        ctrlX, y, editW - 40, CTRL_H, hwnd,
        reinterpret_cast<HMENU>(IDC_SET_BGALPHA), m_hInstance, nullptr);
    SendMessageW(m_bgAlphaSlider, TBM_SETRANGE, TRUE, MAKELPARAM(0, 255));
    SendMessageW(m_bgAlphaSlider, TBM_SETPOS, TRUE, theme.bgImageAlpha);
    m_bgAlphaLabel = makeLabel(L"", ctrlX + editW - 35, y + 2, 35, CTRL_H);
    OnBgAlphaChanged();
    y += CTRL_H + ROW_GAP;

    // CustomIcon
    makeLabel(I18n::Get().T("settings.customicon"), MARGIN, y + 2, LABEL_W, CTRL_H);
    m_customIconEdit = makeEdit(theme.customIcon.c_str(), ctrlX, y, imgEditW);
    makeButton(L"...", IDC_SET_CUSTOMICON_BTN, ctrlX + imgEditW + 4, y, browseW, CTRL_H);
    y += CTRL_H + 12;

    // Separator
    CreateWindowExW(0, L"STATIC", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ,
        MARGIN, y, DLG_W - MARGIN * 2, 2, hwnd, nullptr, m_hInstance, nullptr);
    y += 8;

    // OK / Cancel
    int okX = DLG_W / 2 - BTN_W - 10;
    int cancelX = DLG_W / 2 + 10;
    makeButton(I18n::Get().T("settings.ok"),     IDOK,     okX,     y, BTN_W + 20, BTN_H + 4);
    makeButton(I18n::Get().T("settings.cancel"), IDCANCEL, cancelX, y, BTN_W + 20, BTN_H + 4);
}

void SettingsDialog::OnOpacityChanged() {
    int val = static_cast<int>(SendMessageW(m_opacitySlider, TBM_GETPOS, 0, 0));
    wchar_t buf[16]; wsprintfW(buf, L"%d", val);
    SetWindowTextW(m_opacityLabel, buf);
}

void SettingsDialog::OnBgAlphaChanged() {
    int val = static_cast<int>(SendMessageW(m_bgAlphaSlider, TBM_GETPOS, 0, 0));
    wchar_t buf[16]; wsprintfW(buf, L"%d", val);
    SetWindowTextW(m_bgAlphaLabel, buf);
}

void SettingsDialog::PickColor(HWND hwnd, HWND editCtrl) {
    wchar_t buf[16] = {};
    GetWindowTextW(editCtrl, buf, 16);
    COLORREF current = WStrToColor(buf, RGB(0, 0, 0));

    static COLORREF customColors[16] = {};
    CHOOSECOLORW cc = {};
    cc.lStructSize = sizeof(cc);
    cc.hwndOwner = hwnd;
    cc.rgbResult = current;
    cc.lpCustColors = customColors;
    cc.Flags = CC_FULLOPEN | CC_RGBINIT;

    if (ChooseColorW(&cc)) {
        SetWindowTextW(editCtrl, ColorToWStr(cc.rgbResult).c_str());
    }
}

void SettingsDialog::BrowseFile(HWND hwnd, HWND editCtrl, const wchar_t* filter) {
    wchar_t filePath[MAX_PATH] = {};
    GetWindowTextW(editCtrl, filePath, MAX_PATH);

    OPENFILENAMEW ofn = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwnd;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

    if (GetOpenFileNameW(&ofn)) {
        SetWindowTextW(editCtrl, filePath);
    }
}

void SettingsDialog::OnOK(HWND hwnd) {
    // Language
    int langSel = static_cast<int>(SendMessageW(m_langCombo, CB_GETCURSEL, 0, 0));
    std::wstring langStr = (langSel == 0) ? L"ja" : L"en";
    m_config->SetLanguage(langStr);
    I18n::Get().SetLangFromString(langStr);

    // Hotkey
    int hkSel = static_cast<int>(SendMessageW(m_hotkeyCombo, CB_GETCURSEL, 0, 0));
    UINT mod = (hkSel == 1) ? MOD_CONTROL : MOD_ALT;
    m_config->SetHotKey(mod, VK_SPACE);

    // Opacity
    int opacity = static_cast<int>(SendMessageW(m_opacitySlider, TBM_GETPOS, 0, 0));
    m_config->SetOpacity(static_cast<BYTE>(std::clamp(opacity, 50, 255)));

    // MaxHeight
    { wchar_t buf[16] = {}; GetWindowTextW(m_maxHeightEdit, buf, 16);
      m_config->SetMaxHeight(std::clamp(_wtoi(buf), 200, 2000)); }

    // Checkboxes
    m_config->SetTopmost(SendMessageW(m_topmostCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_config->SetShowSettingsButton(SendMessageW(m_showSettingsCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_config->SetShowHelpButton(SendMessageW(m_showHelpCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);
    m_config->SetHideOnLaunch(SendMessageW(m_hideOnLaunchCheck, BM_GETCHECK, 0, 0) == BST_CHECKED);

    // Theme
    auto& theme = m_config->GetTheme();
    wchar_t buf[MAX_PATH] = {};

    GetWindowTextW(m_titleTextEdit, buf, MAX_PATH);
    theme.titleText = buf;

    GetWindowTextW(m_titleBarColorEdit, buf, 16);
    theme.titleBarColor = WStrToColor(buf, theme.titleBarColor);

    GetWindowTextW(m_titleTextColorEdit, buf, 16);
    theme.titleTextColor = WStrToColor(buf, theme.titleTextColor);

    GetWindowTextW(m_bgColorEdit, buf, 16);
    theme.bgColor = WStrToColor(buf, theme.bgColor);

    GetWindowTextW(m_textColorEdit, buf, 16);
    theme.textColor = WStrToColor(buf, theme.textColor);

    GetWindowTextW(m_selectColorEdit, buf, 16);
    theme.selectColor = WStrToColor(buf, theme.selectColor);

    GetWindowTextW(m_searchBgColorEdit, buf, 16);
    theme.searchBgColor = WStrToColor(buf, theme.searchBgColor);

    GetWindowTextW(m_searchTextColorEdit, buf, 16);
    theme.searchTextColor = WStrToColor(buf, theme.searchTextColor);

    GetWindowTextW(m_bgImageEdit, buf, MAX_PATH);
    theme.bgImage = buf;

    theme.bgImageMode = static_cast<int>(SendMessageW(m_bgModeCombo, CB_GETCURSEL, 0, 0));

    theme.bgImageAlpha = static_cast<BYTE>(
        std::clamp(static_cast<int>(SendMessageW(m_bgAlphaSlider, TBM_GETPOS, 0, 0)), 0, 255));

    GetWindowTextW(m_customIconEdit, buf, MAX_PATH);
    theme.customIcon = buf;

    m_config->Save();
    m_saved = true;
    DestroyWindow(hwnd);
}
