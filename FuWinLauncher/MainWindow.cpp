#define NOMINMAX
#include "MainWindow.h"
#include "SettingsDialog.h"
#include "I18n.h"
#include "resource.h"
#include <windowsx.h>
#include <shellapi.h>
#include <commctrl.h>
#include <algorithm>
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

static const wchar_t* WINDOW_CLASS = L"FuWinLauncherClass";
static constexpr DWORD WND_STYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME;
static constexpr DWORD WND_EX_STYLE_BASE = WS_EX_LAYERED;

MainWindow::~MainWindow() {
    if (m_bgImage) { delete m_bgImage; m_bgImage = nullptr; }
    if (m_fontName) { DeleteObject(m_fontName); m_fontName = nullptr; }
    if (m_gdiplusToken) { Gdiplus::GdiplusShutdown(m_gdiplusToken); m_gdiplusToken = 0; }
}

bool MainWindow::Create(HINSTANCE hInstance, const Config& config) {
    Gdiplus::GdiplusStartupInput gdipInput;
    Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdipInput, nullptr);

    m_hInstance = hInstance;
    m_maxHeight = config.GetMaxHeight();
    m_topmost = config.GetTopmost();
    m_exStyle = WND_EX_STYLE_BASE | (m_topmost ? WS_EX_TOPMOST : 0);
    m_theme = config.GetTheme();

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));
    wc.hIconSm = LoadIconW(hInstance, MAKEINTRESOURCEW(IDI_APP_ICON));
    wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = WINDOW_CLASS;
    RegisterClassExW(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    m_windowWidth = std::min(500, screenW - 100);
    int clientH = CalcClientHeight(DEFAULT_APP_COUNT);

    // Calculate window size from client area (including title bar)
    RECT wr = { 0, 0, static_cast<LONG>(m_windowWidth), clientH };
    AdjustWindowRectEx(&wr, WND_STYLE, FALSE, m_exStyle);
    int windowW = wr.right - wr.left;
    int windowH = wr.bottom - wr.top;

    int posX = (screenW - windowW) / 2;
    int posY = (screenH - windowH) / 2;

    m_hwnd = CreateWindowExW(
        m_exStyle,
        WINDOW_CLASS,
        m_theme.titleText.c_str(),
        WND_STYLE,
        posX, posY, windowW, windowH,
        nullptr, nullptr, hInstance, this
    );

    if (!m_hwnd) return false;

    DragAcceptFiles(m_hwnd, TRUE);
    SetLayeredWindowAttributes(m_hwnd, 0, config.GetOpacity(), LWA_ALPHA);
    ApplyTheme(m_theme);
    LoadBgImage();

    m_fontName = CreateFontW(
        20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_MODERN,
        L"Meiryo UI"
    );

    static constexpr int TOOL_BTN_W = 36;
    static constexpr int TOOL_BTN_GAP = 2;
    bool showSettings = config.GetShowSettingsButton();
    bool showHelp = config.GetShowHelpButton();

    int btnCount = 1;
    if (showSettings) btnCount++;
    if (showHelp) btnCount++;
    int toolbarW = TOOL_BTN_W * btnCount + TOOL_BTN_GAP * (btnCount - 1);
    int searchW = m_windowWidth - SEARCH_MARGIN * 2 - toolbarW - 4;
    m_searchBox.Create(m_hwnd, SEARCH_MARGIN, SEARCH_MARGIN, searchW, SEARCH_HEIGHT);

    {
        // Always create all buttons; hide based on config
        int btnX = m_windowWidth - SEARCH_MARGIN - toolbarW;
        int btnY = SEARCH_MARGIN;

        m_pinButton = CreateWindowExW(
            0, L"BUTTON", L"",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | BS_OWNERDRAW,
            btnX, btnY, TOOL_BTN_W, SEARCH_HEIGHT,
            m_hwnd, reinterpret_cast<HMENU>(IDC_PIN_BUTTON), hInstance, nullptr
        );
        btnX += TOOL_BTN_W + TOOL_BTN_GAP;

        m_settingsButton = CreateWindowExW(
            0, L"BUTTON", L"",
            WS_CHILD | (showSettings ? WS_VISIBLE : 0) | BS_PUSHBUTTON | BS_OWNERDRAW,
            btnX, btnY, TOOL_BTN_W, SEARCH_HEIGHT,
            m_hwnd, reinterpret_cast<HMENU>(IDC_SETTINGS_BUTTON), hInstance, nullptr
        );
        if (showSettings) btnX += TOOL_BTN_W + TOOL_BTN_GAP;

        m_helpButton = CreateWindowExW(
            0, L"BUTTON", L"",
            WS_CHILD | (showHelp ? WS_VISIBLE : 0) | BS_PUSHBUTTON | BS_OWNERDRAW,
            btnX, btnY, TOOL_BTN_W, SEARCH_HEIGHT,
            m_hwnd, reinterpret_cast<HMENU>(IDC_HELP_BUTTON), hInstance, nullptr
        );
    }
    UpdatePinButton();

    return true;
}

void MainWindow::LoadIcons(std::vector<AppEntry>& apps) {
    for (auto& app : apps) {
        if (app.icon) {
            DestroyIcon(app.icon);
            app.icon = nullptr;
        }
        std::wstring expanded = Launcher::ExpandPath(app.path);
        SHFILEINFOW sfi = {};
        if (SHGetFileInfoW(expanded.c_str(), 0, &sfi, sizeof(sfi),
                           SHGFI_ICON | SHGFI_SMALLICON)) {
            app.icon = sfi.hIcon;
        }
    }
}

void MainWindow::Show() {
    m_visible = true;
    m_searchBox.Clear();
    m_selectedIndex = 0;
    m_scrollOffset = 0;
    UpdateFilter();
    ShowWindow(m_hwnd, SW_RESTORE);
    SetForegroundWindow(m_hwnd);
    SetFocus(m_hwnd);
    m_searchBox.SetFocusToEdit();
}

void MainWindow::Hide() {
    m_visible = false;
    ShowWindow(m_hwnd, SW_HIDE);
}

void MainWindow::Toggle() {
    if (m_visible) Hide();
    else Show();
}

void MainWindow::SetApps(std::vector<AppEntry>& apps) {
    LoadIcons(apps);
    m_allApps = &apps;
    ResizeToFit();
    UpdateFilter();
}

void MainWindow::UpdateFilter() {
    if (!m_allApps) return;
    std::wstring query = m_searchBox.GetText();
    m_filtered = SearchBox::Filter(*m_allApps, query);
    m_selectedIndex = 0;
    m_scrollOffset = 0;

    // Adjust window height based on filter results (scroll if exceeding max height)
    int count = static_cast<int>(m_filtered.size());
    if (count < 1) count = 1;
    int clientH = CalcClientHeight(count);
    if (clientH > m_maxHeight) clientH = m_maxHeight;

    RECT wr = { 0, 0, static_cast<LONG>(m_windowWidth), clientH };
    AdjustWindowRectEx(&wr, WND_STYLE, FALSE, m_exStyle);
    int windowH = wr.bottom - wr.top;

    int screenH = GetSystemMetrics(SM_CYSCREEN);
    if (windowH > screenH - 100) windowH = screenH - 100;

    // Keep horizontal position, change height only
    RECT current;
    GetWindowRect(m_hwnd, &current);
    SetWindowPos(m_hwnd, nullptr, current.left, current.top,
                 current.right - current.left, windowH,
                 SWP_NOZORDER | SWP_NOACTIVATE);

    InvalidateRect(m_hwnd, nullptr, TRUE);
}

LRESULT CALLBACK MainWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    MainWindow* self = nullptr;

    if (msg == WM_NCCREATE) {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        self = static_cast<MainWindow*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
        self->m_hwnd = hwnd;
    } else {
        self = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (self) {
        return self->HandleMessage(msg, wParam, lParam);
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT MainWindow::HandleMessage(UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_PAINT:
        OnPaint();
        return 0;

    case WM_KEYDOWN:
        OnKeyDown(wParam);
        return 0;

    case WM_COMMAND:
        OnCommand(wParam, lParam);
        return 0;

    case WM_LBUTTONDOWN: {
        int clickX = GET_X_LPARAM(lParam);
        int clickY = GET_Y_LPARAM(lParam);
        if (IsInScrollbar(clickX) && clickY >= LIST_TOP_MARGIN) {
            m_draggingScrollbar = true;
            m_dragStartY = clickY;
            m_dragStartScroll = m_scrollOffset;
            SetCapture(m_hwnd);
            InvalidateRect(m_hwnd, nullptr, TRUE);
        } else {
            int idx = HitTest(clickY);
            if (idx >= 0 && idx < static_cast<int>(m_filtered.size())) {
                m_selectedIndex = idx;
                // Record as drag candidate (not confirmed until threshold exceeded)
                m_dragItemIndex = idx;
                m_dragStartPt = { clickX, clickY };
                SetCapture(m_hwnd);
                InvalidateRect(m_hwnd, nullptr, TRUE);
            }
        }
        return 0;
    }

    case WM_MOUSEMOVE: {
        int mouseX = GET_X_LPARAM(lParam);
        int mouseY = GET_Y_LPARAM(lParam);
        if (m_draggingScrollbar) {
            RECT rc;
            GetClientRect(m_hwnd, &rc);
            int listH = rc.bottom - LIST_TOP_MARGIN;
            int totalH = static_cast<int>(m_filtered.size()) * ITEM_HEIGHT;
            int maxScroll = totalH - listH;
            if (maxScroll > 0 && listH > 0) {
                int dy = mouseY - m_dragStartY;
                int thumbH = std::max(30, listH * listH / totalH);
                int trackRange = listH - thumbH;
                if (trackRange > 0) {
                    m_scrollOffset = m_dragStartScroll + dy * maxScroll / trackRange;
                    if (m_scrollOffset < 0) m_scrollOffset = 0;
                    if (m_scrollOffset > maxScroll) m_scrollOffset = maxScroll;
                }
            }
            InvalidateRect(m_hwnd, nullptr, TRUE);
        } else if (m_dragItemIndex >= 0) {
            int dx = mouseX - m_dragStartPt.x;
            int dy = mouseY - m_dragStartPt.y;
            if (!m_draggingItem && (dx*dx + dy*dy > DRAG_THRESHOLD * DRAG_THRESHOLD)) {
                m_draggingItem = true;
            }
            if (m_draggingItem) {
                // Calculate insertion position
                int listY = mouseY - LIST_TOP_MARGIN + m_scrollOffset;
                int insertIdx = listY / ITEM_HEIGHT;
                if (listY - insertIdx * ITEM_HEIGHT > ITEM_HEIGHT / 2) insertIdx++;
                if (insertIdx < 0) insertIdx = 0;
                if (insertIdx > static_cast<int>(m_filtered.size()))
                    insertIdx = static_cast<int>(m_filtered.size());
                m_dragInsertIndex = insertIdx;
                InvalidateRect(m_hwnd, nullptr, TRUE);
            }
        } else {
            int idx = HitTest(mouseY);
            if (idx >= 0 && idx < static_cast<int>(m_filtered.size()) && !IsInScrollbar(mouseX)) {
                if (idx != m_selectedIndex) {
                    m_selectedIndex = idx;
                    InvalidateRect(m_hwnd, nullptr, TRUE);
                }
            }
        }
        TRACKMOUSEEVENT tme = {};
        tme.cbSize = sizeof(tme);
        tme.dwFlags = TME_LEAVE;
        tme.hwndTrack = m_hwnd;
        TrackMouseEvent(&tme);
        return 0;
    }

    case WM_LBUTTONUP: {
        if (m_draggingScrollbar) {
            m_draggingScrollbar = false;
            ReleaseCapture();
            InvalidateRect(m_hwnd, nullptr, TRUE);
        } else if (m_draggingItem && m_dragItemIndex >= 0 && m_dragInsertIndex >= 0) {
            // Execute reorder (only when search is not active)
            ReleaseCapture();
            std::wstring query = m_searchBox.GetText();
            if (query.empty() && m_config && m_allApps) {
                int from = m_dragItemIndex;
                int to = m_dragInsertIndex;
                if (to > from) to--;
                if (from != to && from >= 0 && from < static_cast<int>(m_config->GetApps().size())
                    && to >= 0 && to < static_cast<int>(m_config->GetApps().size())) {
                    auto& apps = m_config->GetApps();
                    AppEntry moving = std::move(apps[from]);
                    apps.erase(apps.begin() + from);
                    apps.insert(apps.begin() + to, std::move(moving));
                    SaveAppOrder();
                    SetApps(m_config->GetApps());
                    m_selectedIndex = to;
                }
            }
            m_draggingItem = false;
            m_dragItemIndex = -1;
            m_dragInsertIndex = -1;
            InvalidateRect(m_hwnd, nullptr, TRUE);
        } else if (m_dragItemIndex >= 0) {
            // Below threshold: treat as click and launch app
            ReleaseCapture();
            m_selectedIndex = m_dragItemIndex;
            m_dragItemIndex = -1;
            InvalidateRect(m_hwnd, nullptr, TRUE);
            LaunchSelected();
        }
        return 0;
    }

    case WM_RBUTTONUP: {
        int clickX = GET_X_LPARAM(lParam);
        int clickY = GET_Y_LPARAM(lParam);
        OnRightClick(clickX, clickY);
        return 0;
    }

    case WM_MOUSEWHEEL: {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        m_scrollOffset -= (delta / WHEEL_DELTA) * ITEM_HEIGHT;
        if (m_scrollOffset < 0) m_scrollOffset = 0;
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        int maxScroll = static_cast<int>(m_filtered.size()) * ITEM_HEIGHT -
                        (rc.bottom - LIST_TOP_MARGIN);
        if (maxScroll < 0) maxScroll = 0;
        if (m_scrollOffset > maxScroll) m_scrollOffset = maxScroll;
        InvalidateRect(m_hwnd, nullptr, TRUE);
        return 0;
    }

    case WM_SIZE: {
        if (wParam == SIZE_MINIMIZED) break;
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        static constexpr int TOOL_BTN_W = 36;
        static constexpr int TOOL_BTN_GAP = 2;
        bool hasToolbar = (m_pinButton != nullptr);
        int btnCount = (m_pinButton ? 1 : 0) + (m_settingsButton ? 1 : 0) + (m_helpButton ? 1 : 0);
        int toolbarW = btnCount > 0 ? (TOOL_BTN_W * btnCount + TOOL_BTN_GAP * (btnCount - 1)) : 0;
        int searchW = rc.right - SEARCH_MARGIN * 2 - (hasToolbar ? toolbarW + 4 : 0);
        if (searchW < 100) searchW = 100;
        if (m_searchBox.GetHWND()) {
            SetWindowPos(m_searchBox.GetHWND(), nullptr,
                         SEARCH_MARGIN, SEARCH_MARGIN, searchW, SEARCH_HEIGHT,
                         SWP_NOZORDER);
        }
        if (hasToolbar) {
            int btnX = rc.right - SEARCH_MARGIN - toolbarW;
            if (m_pinButton) {
                SetWindowPos(m_pinButton, nullptr, btnX, SEARCH_MARGIN,
                             TOOL_BTN_W, SEARCH_HEIGHT, SWP_NOZORDER);
                btnX += TOOL_BTN_W + TOOL_BTN_GAP;
            }
            if (m_settingsButton) {
                SetWindowPos(m_settingsButton, nullptr, btnX, SEARCH_MARGIN,
                             TOOL_BTN_W, SEARCH_HEIGHT, SWP_NOZORDER);
                btnX += TOOL_BTN_W + TOOL_BTN_GAP;
            }
            if (m_helpButton) {
                SetWindowPos(m_helpButton, nullptr, btnX, SEARCH_MARGIN,
                             TOOL_BTN_W, SEARCH_HEIGHT, SWP_NOZORDER);
            }
        }
        InvalidateRect(m_hwnd, nullptr, TRUE);
        return 0;
    }

    case WM_GETMINMAXINFO: {
        auto mmi = reinterpret_cast<MINMAXINFO*>(lParam);
        RECT minRect = { 0, 0, 300, CalcClientHeight(3) };
        AdjustWindowRectEx(&minRect, WND_STYLE, FALSE, m_exStyle);
        mmi->ptMinTrackSize.x = minRect.right - minRect.left;
        mmi->ptMinTrackSize.y = minRect.bottom - minRect.top;
        return 0;
    }

    case WM_DRAWITEM: {
        auto dis = reinterpret_cast<DRAWITEMSTRUCT*>(lParam);
        auto drawToolButton = [&](COLORREF bg, COLORREF border, const wchar_t* icon) {
            HBRUSH br = CreateSolidBrush(bg);
            FillRect(dis->hDC, &dis->rcItem, br);
            DeleteObject(br);
            HPEN pen = CreatePen(PS_SOLID, 1, border);
            HPEN oldPen = static_cast<HPEN>(SelectObject(dis->hDC, pen));
            HBRUSH oldBr = static_cast<HBRUSH>(SelectObject(dis->hDC, GetStockObject(NULL_BRUSH)));
            Rectangle(dis->hDC, dis->rcItem.left, dis->rcItem.top, dis->rcItem.right, dis->rcItem.bottom);
            SelectObject(dis->hDC, oldPen);
            SelectObject(dis->hDC, oldBr);
            DeleteObject(pen);
            SetBkMode(dis->hDC, TRANSPARENT);
            SetTextColor(dis->hDC, RGB(255, 255, 255));
            HFONT btnFont = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Segoe UI Emoji");
            HFONT oldFont = static_cast<HFONT>(SelectObject(dis->hDC, btnFont));
            DrawTextW(dis->hDC, icon, -1, &dis->rcItem,
                      DT_CENTER | DT_SINGLELINE | DT_VCENTER);
            SelectObject(dis->hDC, oldFont);
            DeleteObject(btnFont);
        };

        if (dis->CtlID == IDC_PIN_BUTTON) {
            COLORREF bg = m_topmost ? RGB(80, 140, 220) : RGB(60, 60, 70);
            COLORREF border = m_topmost ? RGB(100, 170, 255) : RGB(90, 90, 100);
            drawToolButton(bg, border, L"\xD83D\xDCCC");  // 📌
            return TRUE;
        }
        if (dis->CtlID == IDC_SETTINGS_BUTTON) {
            drawToolButton(RGB(50, 55, 75), RGB(100, 110, 140), L"\x2699");  // ⚙
            return TRUE;
        }
        if (dis->CtlID == IDC_HELP_BUTTON) {
            drawToolButton(RGB(50, 55, 75), RGB(100, 110, 140), L"\x2139");  // ℹ
            return TRUE;
        }
        break;
    }

    case WM_CTLCOLOREDIT: {
        HDC hdc = reinterpret_cast<HDC>(wParam);
        HWND ctrl = reinterpret_cast<HWND>(lParam);
        if (ctrl == m_searchBox.GetHWND()) {
            SetTextColor(hdc, m_searchBox.GetTextColor());
            SetBkColor(hdc, m_theme.searchBgColor);
            return reinterpret_cast<LRESULT>(m_searchBox.GetBgBrush());
        }
        break;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DROPFILES:
        OnDropFiles(reinterpret_cast<HDROP>(wParam));
        return 0;

    case WM_CLOSE:
        Hide();
        return 0;

    case WM_TRAYICON: {
        UINT trayMsg = LOWORD(lParam);
        if (trayMsg == WM_LBUTTONUP) {
            Show();
        } else if (trayMsg == WM_RBUTTONUP) {
            HMENU menu = CreatePopupMenu();
            AppendMenuW(menu, MF_STRING, IDM_TRAY_SHOW, I18n::Get().T("tray.show"));
            AppendMenuW(menu, MF_STRING, IDM_TRAY_SETTINGS, I18n::Get().T("tray.settings"));
            AppendMenuW(menu, MF_STRING, IDM_TRAY_HELP, I18n::Get().T("tray.help"));
            AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(menu, MF_STRING, IDM_TRAY_EXIT, I18n::Get().T("tray.exit"));
            POINT pt;
            GetCursorPos(&pt);
            SetForegroundWindow(m_hwnd);
            UINT cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY,
                                      pt.x, pt.y, 0, m_hwnd, nullptr);
            DestroyMenu(menu);
            if (cmd == IDM_TRAY_SHOW) {
                Show();
            } else if (cmd == IDM_TRAY_SETTINGS) {
                if (SettingsDialog::Show(m_hwnd, m_hInstance, *m_config)) {
                    ReloadFromConfig();
                }
            } else if (cmd == IDM_TRAY_HELP) {
                MessageBoxW(m_hwnd, I18n::Get().T("help.text"),
                            I18n::Get().T("help.title"), MB_OK | MB_ICONINFORMATION);
            } else if (cmd == IDM_TRAY_EXIT) {
                DestroyWindow(m_hwnd);
            }
        }
        return 0;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(m_hwnd, msg, wParam, lParam);
}

void MainWindow::OnPaint() {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(m_hwnd, &ps);

    RECT rc;
    GetClientRect(m_hwnd, &rc);

    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBmp = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    HBITMAP oldBmp = static_cast<HBITMAP>(SelectObject(memDC, memBmp));

    HBRUSH bgBrush = CreateSolidBrush(m_theme.bgColor);
    FillRect(memDC, &rc, bgBrush);
    DeleteObject(bgBrush);

    // Draw background image if loaded
    if (m_bgImage) {
        Gdiplus::Graphics gfx(memDC);
        int imgW = m_bgImage->GetWidth();
        int imgH = m_bgImage->GetHeight();
        int areaW = rc.right;
        int areaH = rc.bottom - LIST_TOP_MARGIN;

        float alpha = m_theme.bgImageAlpha / 255.0f;
        Gdiplus::ColorMatrix cm = {
            1, 0, 0, 0, 0,
            0, 1, 0, 0, 0,
            0, 0, 1, 0, 0,
            0, 0, 0, alpha, 0,
            0, 0, 0, 0, 1
        };
        Gdiplus::ImageAttributes attr;
        attr.SetColorMatrix(&cm);

        switch (m_theme.bgImageMode) {
        case 1: // Stretch
            gfx.DrawImage(m_bgImage,
                Gdiplus::Rect(0, LIST_TOP_MARGIN, areaW, areaH),
                0, 0, imgW, imgH,
                Gdiplus::UnitPixel, &attr);
            break;
        case 2: // Tile
            for (int ty = LIST_TOP_MARGIN; ty < rc.bottom; ty += imgH) {
                for (int tx = 0; tx < areaW; tx += imgW) {
                    gfx.DrawImage(m_bgImage,
                        Gdiplus::Rect(tx, ty, imgW, imgH),
                        0, 0, imgW, imgH,
                        Gdiplus::UnitPixel, &attr);
                }
            }
            break;
        default: // Center (0)
            gfx.DrawImage(m_bgImage,
                Gdiplus::Rect((areaW - imgW) / 2, LIST_TOP_MARGIN + (areaH - imgH) / 2, imgW, imgH),
                0, 0, imgW, imgH,
                Gdiplus::UnitPixel, &attr);
            break;
        }
    }

    SetBkMode(memDC, TRANSPARENT);

    // Set clipping to list area (prevent drawing behind search box)
    HRGN clipRgn = CreateRectRgn(0, LIST_TOP_MARGIN, rc.right, rc.bottom);
    SelectClipRgn(memDC, clipRgn);

    int y = LIST_TOP_MARGIN - m_scrollOffset;
    for (int i = 0; i < static_cast<int>(m_filtered.size()); i++) {
        if (y + ITEM_HEIGHT < LIST_TOP_MARGIN) { y += ITEM_HEIGHT; continue; }
        if (y > rc.bottom) break;

        if (m_draggingItem && i == m_dragItemIndex) {
            // Dim the item being dragged
            HBRUSH dimBrush = CreateSolidBrush(RGB(40, 40, 55));
            RECT dimRect = { 0, y, rc.right, y + ITEM_HEIGHT };
            FillRect(memDC, &dimRect, dimBrush);
            DeleteObject(dimBrush);
        } else if (i == m_selectedIndex) {
            HBRUSH selBrush = CreateSolidBrush(m_theme.selectColor);
            RECT highlightRect = { 0, y, rc.right, y + ITEM_HEIGHT };
            FillRect(memDC, &highlightRect, selBrush);
            DeleteObject(selBrush);
        }

        int iconX = ITEM_PADDING + 4;
        int iconY = y + (ITEM_HEIGHT - ICON_SIZE) / 2;
        if (m_filtered[i]->icon) {
            DrawIconEx(memDC, iconX, iconY, m_filtered[i]->icon,
                       ICON_SIZE, ICON_SIZE, 0, nullptr, DI_NORMAL);
        }

        HFONT oldFont = static_cast<HFONT>(SelectObject(memDC, m_fontName));
        SetTextColor(memDC, m_theme.textColor);
        int textX = iconX + ICON_SIZE + 10;
        RECT nameRect = { textX, y, rc.right - ITEM_PADDING, y + ITEM_HEIGHT };
        DrawTextW(memDC, m_filtered[i]->name.c_str(), -1, &nameRect,
                  DT_LEFT | DT_SINGLELINE | DT_VCENTER | DT_END_ELLIPSIS);
        SelectObject(memDC, oldFont);

        y += ITEM_HEIGHT;
    }

    // Drag insertion indicator
    if (m_draggingItem && m_dragInsertIndex >= 0) {
        int lineY = LIST_TOP_MARGIN + m_dragInsertIndex * ITEM_HEIGHT - m_scrollOffset;
        if (lineY >= LIST_TOP_MARGIN && lineY <= rc.bottom) {
            HPEN linePen = CreatePen(PS_SOLID, 3, RGB(100, 180, 255));
            HPEN oldPen = static_cast<HPEN>(SelectObject(memDC, linePen));
            MoveToEx(memDC, ITEM_PADDING, lineY, nullptr);
            LineTo(memDC, rc.right - ITEM_PADDING - SCROLLBAR_WIDTH, lineY);
            SelectObject(memDC, oldPen);
            DeleteObject(linePen);
        }
    }

    // Remove clipping
    SelectClipRgn(memDC, nullptr);
    DeleteObject(clipRgn);

    if (m_filtered.empty() && m_allApps && !m_allApps->empty()) {
        SetTextColor(memDC, RGB(120, 120, 140));
        HFONT oldFont = static_cast<HFONT>(SelectObject(memDC, m_fontName));
        RECT hintRect = { 0, rc.bottom / 2 - 20, rc.right, rc.bottom / 2 + 20 };
        DrawTextW(memDC, I18n::Get().T("main.no_match"), -1, &hintRect,
                  DT_CENTER | DT_SINGLELINE | DT_VCENTER);
        SelectObject(memDC, oldFont);
    }

    // Draw scrollbar
    int totalH = static_cast<int>(m_filtered.size()) * ITEM_HEIGHT;
    int listH = rc.bottom - LIST_TOP_MARGIN;
    if (totalH > listH && listH > 0) {
        int trackX = rc.right - SCROLLBAR_WIDTH;
        int thumbH = std::max(30, listH * listH / totalH);
        int maxScroll = totalH - listH;
        int thumbY = LIST_TOP_MARGIN + (maxScroll > 0 ? m_scrollOffset * (listH - thumbH) / maxScroll : 0);

        // Track background
        HBRUSH trackBr = CreateSolidBrush(RGB(50, 50, 60));
        RECT trackRect = { trackX, LIST_TOP_MARGIN, rc.right, rc.bottom };
        FillRect(memDC, &trackRect, trackBr);
        DeleteObject(trackBr);

        // Thumb (slightly padded for a rounded look)
        HBRUSH thumbBr = CreateSolidBrush(m_draggingScrollbar ? RGB(160, 170, 200) : RGB(120, 130, 160));
        RECT thumbRect = { trackX + 2, thumbY, rc.right - 2, thumbY + thumbH };
        FillRect(memDC, &thumbRect, thumbBr);
        DeleteObject(thumbBr);
    }

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBmp);
    DeleteObject(memBmp);
    DeleteDC(memDC);

    EndPaint(m_hwnd, &ps);
}

void MainWindow::OnKeyDown(WPARAM vk) {
    switch (vk) {
    case VK_ESCAPE:
        Hide();
        break;

    case VK_UP:
        if (m_selectedIndex > 0) {
            m_selectedIndex--;
            EnsureVisible(m_selectedIndex);
            InvalidateRect(m_hwnd, nullptr, TRUE);
        }
        break;

    case VK_DOWN:
        if (m_selectedIndex < static_cast<int>(m_filtered.size()) - 1) {
            m_selectedIndex++;
            EnsureVisible(m_selectedIndex);
            InvalidateRect(m_hwnd, nullptr, TRUE);
        }
        break;

    case VK_RETURN:
        LaunchSelected();
        break;
    }
}

void MainWindow::OnCommand(WPARAM wParam, LPARAM /*lParam*/) {
    if (LOWORD(wParam) == IDC_SEARCH_EDIT && HIWORD(wParam) == EN_CHANGE) {
        UpdateFilter();
    }
    if (LOWORD(wParam) == IDC_PIN_BUTTON && HIWORD(wParam) == BN_CLICKED) {
        ToggleTopmost();
    }
    if (LOWORD(wParam) == IDC_SETTINGS_BUTTON && HIWORD(wParam) == BN_CLICKED) {
        if (SettingsDialog::Show(m_hwnd, m_hInstance, *m_config)) {
            ReloadFromConfig();
        }
    }
    if (LOWORD(wParam) == IDC_HELP_BUTTON && HIWORD(wParam) == BN_CLICKED) {
        ShowHelp();
    }
}

void MainWindow::ToggleTopmost() {
    m_topmost = !m_topmost;
    m_exStyle = WND_EX_STYLE_BASE | (m_topmost ? WS_EX_TOPMOST : 0);
    SetWindowPos(m_hwnd, m_topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    UpdatePinButton();

}

void MainWindow::UpdatePinButton() {
    if (!m_pinButton) return;
    InvalidateRect(m_pinButton, nullptr, TRUE);
}

void MainWindow::OnDropFiles(HDROP hDrop) {
    if (!m_config) {
        DragFinish(hDrop);
        return;
    }

    UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
    int added = 0;

    for (UINT i = 0; i < count; i++) {
        wchar_t filePath[MAX_PATH] = {};
        DragQueryFileW(hDrop, i, filePath, MAX_PATH);

        std::wstring path(filePath);
        std::wstring ext;
        size_t dot = path.rfind(L'.');
        if (dot != std::wstring::npos) {
            ext = path.substr(dot);
            for (auto& c : ext) c = towlower(c);
        }

        // Only accept .exe and .lnk files
        if (ext != L".exe" && ext != L".lnk") continue;

        // Use filename (without extension) as app name
        size_t lastSlash = path.find_last_of(L"\\/");
        std::wstring fileName = (lastSlash != std::wstring::npos)
            ? path.substr(lastSlash + 1) : path;
        size_t dotPos = fileName.rfind(L'.');
        std::wstring name = (dotPos != std::wstring::npos)
            ? fileName.substr(0, dotPos) : fileName;

        if (m_config->AppendApp(name, path)) {
            added++;
        }
    }

    DragFinish(hDrop);

    if (added > 0) {
        // Reload icons and update list
        SetApps(m_config->GetApps());
    }
}

void MainWindow::LaunchSelected() {
    if (m_selectedIndex >= 0 && m_selectedIndex < static_cast<int>(m_filtered.size())) {
        Launcher::Launch(m_filtered[m_selectedIndex]->path);
        if (m_config && m_config->GetHideOnLaunch()) {
            Hide();
        } else {
            // Reset search state so the launcher is ready for the next pick
            m_searchBox.Clear();
            m_selectedIndex = 0;
            m_scrollOffset = 0;
            UpdateFilter();
            m_searchBox.SetFocusToEdit();
        }
    }
}

int MainWindow::HitTest(int y) const {
    int listY = y - LIST_TOP_MARGIN + m_scrollOffset;
    if (listY < 0) return -1;
    return listY / ITEM_HEIGHT;
}

void MainWindow::OnRightClick(int x, int y) {
    if (IsInScrollbar(x) || y < LIST_TOP_MARGIN) return;

    std::wstring query = m_searchBox.GetText();
    if (!query.empty()) return;

    int idx = HitTest(y);
    bool onItem = (idx >= 0 && idx < static_cast<int>(m_filtered.size()));

    if (onItem) {
        m_selectedIndex = idx;
        InvalidateRect(m_hwnd, nullptr, TRUE);
    }

    HMENU menu = CreatePopupMenu();
    if (onItem) {
        AppendMenuW(menu, MF_STRING, IDM_APP_EDIT, I18n::Get().T("menu.edit"));
        AppendMenuW(menu, MF_STRING, IDM_APP_DELETE, I18n::Get().T("menu.delete"));
        AppendMenuW(menu, MF_SEPARATOR, 0, nullptr);
    }
    AppendMenuW(menu, MF_STRING, IDM_APP_ADD, I18n::Get().T("menu.add"));

    POINT pt = { x, y };
    ClientToScreen(m_hwnd, &pt);
    UINT cmd = TrackPopupMenu(menu, TPM_RETURNCMD | TPM_NONOTIFY,
                              pt.x, pt.y, 0, m_hwnd, nullptr);
    DestroyMenu(menu);

    if (cmd == IDM_APP_EDIT && onItem) {
        EditApp(idx);
    } else if (cmd == IDM_APP_DELETE && onItem) {
        DeleteApp(idx);
    } else if (cmd == IDM_APP_ADD) {
        AddNewApp();
    }
}

// Help dialog state
struct HelpDlgState { bool closed = false; };
static HelpDlgState* s_helpState = nullptr;

static LRESULT CALLBACK HelpDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        s_helpState = static_cast<HelpDlgState*>(cs->lpCreateParams);

        HFONT font = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Meiryo UI");

        HINSTANCE hInst = GetModuleHandleW(nullptr);

        // Measure help text height
        const wchar_t* helpText = I18n::Get().T("help.text");
        RECT clientRc;
        GetClientRect(hwnd, &clientRc);
        int contentW = clientRc.right - 24;

        HDC hdc = GetDC(hwnd);
        HFONT oldFont = static_cast<HFONT>(SelectObject(hdc, font));
        RECT measureRc = { 0, 0, contentW, 0 };
        DrawTextW(hdc, helpText, -1, &measureRc, DT_CALCRECT | DT_WORDBREAK);
        int textH = measureRc.bottom + 4;
        SelectObject(hdc, oldFont);
        ReleaseDC(hwnd, hdc);

        // Help text (static)
        HWND textCtrl = CreateWindowExW(0, L"STATIC", helpText,
            WS_CHILD | WS_VISIBLE,
            12, 10, contentW, textH,
            hwnd, nullptr, hInst, nullptr);
        SendMessageW(textCtrl, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

        int btnY = 10 + textH + 8;

        // Ko-fi button
        const wchar_t* kofiLabel = (I18n::Get().GetLang() == Lang::JA)
            ? L"\x2764 Ko-fi \x3067\x5FDC\x63F4\x3059\x308B" : L"\x2764 Support on Ko-fi";
        HWND kofiBtn = CreateWindowExW(0, L"BUTTON", kofiLabel,
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            12, btnY, 200, 30,
            hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(200)), hInst, nullptr);
        SendMessageW(kofiBtn, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

        int okY = btnY + 40;

        // OK button
        HWND okBtn = CreateWindowExW(0, L"BUTTON", L"OK",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            170, okY, 80, 30,
            hwnd, reinterpret_cast<HMENU>(IDOK), hInst, nullptr);
        SendMessageW(okBtn, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

        // Resize dialog to fit content
        int neededH = okY + 30 + 16;
        RECT wr2 = { 0, 0, 420, neededH };
        AdjustWindowRectEx(&wr2, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE, 0);
        SetWindowPos(hwnd, nullptr, 0, 0, wr2.right - wr2.left, wr2.bottom - wr2.top,
                     SWP_NOMOVE | SWP_NOZORDER);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == 200) {
            ShellExecuteW(nullptr, L"open", L"https://ko-fi.com/fufoo", nullptr, nullptr, SW_SHOWNORMAL);
        }
        if (LOWORD(wParam) == IDOK) DestroyWindow(hwnd);
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        if (s_helpState) s_helpState->closed = true;
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void MainWindow::ShowHelp() {
    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = HelpDlgProc;
        wc.hInstance = m_hInstance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = L"FuWinHelpDlg";
        RegisterClassExW(&wc);
        classRegistered = true;
    }

    HelpDlgState state;

    RECT wr = { 0, 0, 420, 400 };
    AdjustWindowRectEx(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE, 0);
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    HWND dlg = CreateWindowExW(0, L"FuWinHelpDlg", I18n::Get().T("help.title"),
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        (screenW - (wr.right - wr.left)) / 2, (screenH - (wr.bottom - wr.top)) / 2,
        wr.right - wr.left, wr.bottom - wr.top,
        m_hwnd, nullptr, m_hInstance, &state);

    EnableWindow(m_hwnd, FALSE);
    ShowWindow(dlg, SW_SHOW);

    MSG msg;
    while (!state.closed && GetMessageW(&msg, nullptr, 0, 0)) {
        if (IsDialogMessageW(dlg, &msg)) continue;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    EnableWindow(m_hwnd, TRUE);
    SetForegroundWindow(m_hwnd);
}

void MainWindow::DeleteApp(int index) {
    if (!m_config || index < 0 || index >= static_cast<int>(m_config->GetApps().size())) return;

    auto& apps = m_config->GetApps();
    if (apps[index].icon) {
        DestroyIcon(apps[index].icon);
    }
    apps.erase(apps.begin() + index);
    m_config->Save();
    SetApps(m_config->GetApps());
}

// Simple edit dialog state
struct EditDlgState {
    std::wstring name;
    std::wstring path;
    bool ok = false;
    bool closed = false;
};

static EditDlgState* s_editState = nullptr;

static LRESULT CALLBACK EditDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
        s_editState = static_cast<EditDlgState*>(cs->lpCreateParams);

        HFONT font = CreateFontW(15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, 0, 0, CLEARTYPE_QUALITY, 0, L"Meiryo UI");

        auto mk = [&](const wchar_t* cls, const wchar_t* text, DWORD style, int x, int y, int w, int h, int id = 0) {
            HWND h_ = CreateWindowExW(
                (cls == std::wstring(L"EDIT")) ? WS_EX_CLIENTEDGE : 0,
                cls, text, WS_CHILD | WS_VISIBLE | style,
                x, y, w, h, hwnd, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
                GetModuleHandleW(nullptr), nullptr);
            SendMessageW(h_, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            return h_;
        };

        mk(L"STATIC", I18n::Get().T("edit.name"), 0, 10, 12, 60, 22);
        HWND nameEdit = mk(L"EDIT", s_editState->name.c_str(), ES_AUTOHSCROLL | WS_TABSTOP, 75, 10, 270, 24, 100);

        mk(L"STATIC", I18n::Get().T("edit.path"), 0, 10, 44, 60, 22);
        mk(L"EDIT", s_editState->path.c_str(), ES_AUTOHSCROLL | WS_TABSTOP, 75, 42, 270, 24, 101);

        mk(L"BUTTON", L"OK", BS_PUSHBUTTON | WS_TABSTOP, 190, 78, 70, 28, IDOK);
        mk(L"BUTTON", I18n::Get().T("settings.cancel"), BS_PUSHBUTTON | WS_TABSTOP, 270, 78, 70, 28, IDCANCEL);

        SetFocus(nameEdit);
        return 0;
    }
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK) {
            wchar_t buf[MAX_PATH] = {};
            GetWindowTextW(GetDlgItem(hwnd, 100), buf, MAX_PATH);
            s_editState->name = buf;
            GetWindowTextW(GetDlgItem(hwnd, 101), buf, MAX_PATH);
            s_editState->path = buf;
            s_editState->ok = true;
            DestroyWindow(hwnd);
        } else if (LOWORD(wParam) == IDCANCEL) {
            DestroyWindow(hwnd);
        }
        return 0;
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;
    case WM_DESTROY:
        if (s_editState) s_editState->closed = true;
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

bool MainWindow::ShowEditDialog(const wchar_t* title, std::wstring& name, std::wstring& path) {
    EditDlgState state;
    state.name = name;
    state.path = path;

    static bool classRegistered = false;
    if (!classRegistered) {
        WNDCLASSEXW wc = {};
        wc.cbSize = sizeof(wc);
        wc.lpfnWndProc = EditDlgProc;
        wc.hInstance = m_hInstance;
        wc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        wc.lpszClassName = L"FuWinEditDlg";
        RegisterClassExW(&wc);
        classRegistered = true;
    }

    RECT wr = { 0, 0, 360, 118 };
    AdjustWindowRectEx(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, FALSE, 0);
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);

    HWND dlg = CreateWindowExW(0, L"FuWinEditDlg", title,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU,
        (screenW - (wr.right - wr.left)) / 2, (screenH - (wr.bottom - wr.top)) / 2,
        wr.right - wr.left, wr.bottom - wr.top,
        m_hwnd, nullptr, m_hInstance, &state);

    EnableWindow(m_hwnd, FALSE);
    ShowWindow(dlg, SW_SHOW);

    MSG msg;
    while (!state.closed && GetMessageW(&msg, nullptr, 0, 0)) {
        if (IsDialogMessageW(dlg, &msg)) continue;
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    EnableWindow(m_hwnd, TRUE);
    SetForegroundWindow(m_hwnd);

    if (state.ok && !state.name.empty() && !state.path.empty()) {
        name = state.name;
        path = state.path;
        return true;
    }
    return false;
}

void MainWindow::EditApp(int index) {
    if (!m_config || index < 0 || index >= static_cast<int>(m_config->GetApps().size())) return;

    auto& app = m_config->GetApps()[index];
    std::wstring name = app.name;
    std::wstring path = app.path;

    if (ShowEditDialog(I18n::Get().T("edit.title"), name, path)) {
        app.name = name;
        app.path = path;
        m_config->Save();
        SetApps(m_config->GetApps());
    }
}

void MainWindow::AddNewApp() {
    if (!m_config) return;

    std::wstring name;
    std::wstring path;

    if (ShowEditDialog(I18n::Get().T("add.title"), name, path)) {
        AppEntry entry;
        entry.name = name;
        entry.path = path;
        m_config->GetApps().push_back(std::move(entry));
        m_config->Save();
        SetApps(m_config->GetApps());
    }
}

void MainWindow::ReloadFromConfig() {
    if (!m_config) return;

    // Reapply skin if configured (overrides theme)
    if (!m_config->GetSkin().empty()) {
        std::wstring iniPath = m_config->GetIniPath();
        std::wstring exeDir = iniPath.substr(0, iniPath.find_last_of(L"\\/") + 1);
        m_config->LoadSkin(m_config->GetSkin(), exeDir);
    }

    // Reapply opacity
    SetLayeredWindowAttributes(m_hwnd, 0, m_config->GetOpacity(), LWA_ALPHA);

    // Reapply topmost
    m_topmost = m_config->GetTopmost();
    m_exStyle = WND_EX_STYLE_BASE | (m_topmost ? WS_EX_TOPMOST : 0);
    SetWindowPos(m_hwnd, m_topmost ? HWND_TOPMOST : HWND_NOTOPMOST,
                 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    UpdatePinButton();

    // Reapply max height
    m_maxHeight = m_config->GetMaxHeight();

    // Re-register hotkey
    UnregisterHotKey(m_hwnd, IDH_HOTKEY);
    RegisterHotKey(m_hwnd, IDH_HOTKEY,
                   m_config->GetHotKeyModifiers() | MOD_NOREPEAT,
                   m_config->GetHotKeyVK());

    // Reapply theme
    ApplyTheme(m_config->GetTheme());
    LoadBgImage();

    // Show/hide settings and help buttons
    if (m_settingsButton) {
        ShowWindow(m_settingsButton, m_config->GetShowSettingsButton() ? SW_SHOW : SW_HIDE);
    }
    if (m_helpButton) {
        ShowWindow(m_helpButton, m_config->GetShowHelpButton() ? SW_SHOW : SW_HIDE);
    }

    // Recalculate toolbar layout
    {
        static constexpr int TOOL_BTN_W = 36;
        static constexpr int TOOL_BTN_GAP = 2;
        RECT rc;
        GetClientRect(m_hwnd, &rc);
        int btnCount = 1;  // pin always
        if (m_settingsButton && m_config->GetShowSettingsButton()) btnCount++;
        if (m_helpButton && m_config->GetShowHelpButton()) btnCount++;
        int toolbarW = TOOL_BTN_W * btnCount + TOOL_BTN_GAP * (btnCount - 1);
        int searchW = rc.right - SEARCH_MARGIN * 2 - toolbarW - 4;
        if (searchW < 100) searchW = 100;
        if (m_searchBox.GetHWND()) {
            SetWindowPos(m_searchBox.GetHWND(), nullptr,
                         SEARCH_MARGIN, SEARCH_MARGIN, searchW, SEARCH_HEIGHT, SWP_NOZORDER);
        }
        int btnX = rc.right - SEARCH_MARGIN - toolbarW;
        if (m_pinButton) {
            SetWindowPos(m_pinButton, nullptr, btnX, SEARCH_MARGIN,
                         TOOL_BTN_W, SEARCH_HEIGHT, SWP_NOZORDER);
            btnX += TOOL_BTN_W + TOOL_BTN_GAP;
        }
        if (m_settingsButton && m_config->GetShowSettingsButton()) {
            SetWindowPos(m_settingsButton, nullptr, btnX, SEARCH_MARGIN,
                         TOOL_BTN_W, SEARCH_HEIGHT, SWP_NOZORDER);
            btnX += TOOL_BTN_W + TOOL_BTN_GAP;
        }
        if (m_helpButton && m_config->GetShowHelpButton()) {
            SetWindowPos(m_helpButton, nullptr, btnX, SEARCH_MARGIN,
                         TOOL_BTN_W, SEARCH_HEIGHT, SWP_NOZORDER);
        }
    }

    // Reload app list
    SetApps(m_config->GetApps());
}

void MainWindow::SaveAppOrder() {
    if (m_config) {
        m_config->SaveApps();
    }
}

void MainWindow::ApplyTheme(const ThemeConfig& theme) {
    m_theme = theme;
    SetWindowTextW(m_hwnd, m_theme.titleText.c_str());
    m_searchBox.SetColors(m_theme.searchBgColor, m_theme.searchTextColor);

    // Apply title bar color using DWM (Windows 11+)
    HMODULE dwm = LoadLibraryW(L"dwmapi.dll");
    if (dwm) {
        using DwmSetAttrFunc = HRESULT(WINAPI*)(HWND, DWORD, LPCVOID, DWORD);
        auto pDwmSetAttr = reinterpret_cast<DwmSetAttrFunc>(
            GetProcAddress(dwm, "DwmSetWindowAttribute"));
        if (pDwmSetAttr) {
            // DWMWA_CAPTION_COLOR = 35, DWMWA_TEXT_COLOR = 36
            COLORREF captionColor = m_theme.titleBarColor;
            pDwmSetAttr(m_hwnd, 35, &captionColor, sizeof(captionColor));
            COLORREF textColor = m_theme.titleTextColor;
            pDwmSetAttr(m_hwnd, 36, &textColor, sizeof(textColor));
        }
        FreeLibrary(dwm);
    }

    // Custom icon
    if (!m_theme.customIcon.empty()) {
        HICON hIcon = static_cast<HICON>(LoadImageW(nullptr, m_theme.customIcon.c_str(),
            IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE));
        if (hIcon) {
            SendMessageW(m_hwnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
            SendMessageW(m_hwnd, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIcon));
        }
    }

    InvalidateRect(m_hwnd, nullptr, TRUE);
}

void MainWindow::LoadBgImage() {
    if (m_bgImage) { delete m_bgImage; m_bgImage = nullptr; }
    if (!m_theme.bgImage.empty()) {
        auto* bmp = new Gdiplus::Bitmap(m_theme.bgImage.c_str());
        if (bmp->GetLastStatus() == Gdiplus::Ok) {
            m_bgImage = bmp;
        } else {
            delete bmp;
        }
    }
}

void MainWindow::EnsureVisible(int index) {
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    int listH = rc.bottom - LIST_TOP_MARGIN;

    int itemTop = index * ITEM_HEIGHT;
    int itemBottom = itemTop + ITEM_HEIGHT;

    if (itemTop < m_scrollOffset) {
        m_scrollOffset = itemTop;
    } else if (itemBottom > m_scrollOffset + listH) {
        m_scrollOffset = itemBottom - listH;
    }
}

bool MainWindow::IsInScrollbar(int x) const {
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    return x >= rc.right - SCROLLBAR_WIDTH;
}

int MainWindow::GetMaxScroll() const {
    RECT rc;
    GetClientRect(m_hwnd, &rc);
    int listH = rc.bottom - LIST_TOP_MARGIN;
    int totalH = static_cast<int>(m_filtered.size()) * ITEM_HEIGHT;
    int ms = totalH - listH;
    return ms > 0 ? ms : 0;
}

int MainWindow::CalcClientHeight(int appCount) const {
    if (appCount < 1) appCount = 1;
    return LIST_TOP_MARGIN + ITEM_HEIGHT * appCount + BOTTOM_MARGIN;
}

void MainWindow::ResizeToFit() {
    if (!m_allApps || !m_hwnd) return;

    int appCount = static_cast<int>(m_allApps->size());
    if (appCount < 1) appCount = DEFAULT_APP_COUNT;

    int clientH = CalcClientHeight(appCount);
    if (clientH > m_maxHeight) clientH = m_maxHeight;

    RECT wr = { 0, 0, static_cast<LONG>(m_windowWidth), clientH };
    AdjustWindowRectEx(&wr, WND_STYLE, FALSE, m_exStyle);
    int windowW = wr.right - wr.left;
    int windowH = wr.bottom - wr.top;

    int screenH = GetSystemMetrics(SM_CYSCREEN);
    if (windowH > screenH - 100) windowH = screenH - 100;

    // Keep current position, only change size
    RECT current;
    GetWindowRect(m_hwnd, &current);
    SetWindowPos(m_hwnd, nullptr, current.left, current.top, windowW, windowH,
                 SWP_NOZORDER | SWP_NOACTIVATE);
}
