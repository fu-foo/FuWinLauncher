#include "I18n.h"

I18n& I18n::Get() {
    static I18n instance;
    return instance;
}

I18n::I18n() {
    Init();
}

void I18n::DetectFromOS() {
    LANGID langId = GetUserDefaultUILanguage();
    WORD primary = PRIMARYLANGID(langId);
    m_lang = (primary == LANG_JAPANESE) ? Lang::JA : Lang::EN;
}

void I18n::SetLangFromString(const std::wstring& str) {
    if (str == L"ja" || str == L"JA") m_lang = Lang::JA;
    else if (str == L"en" || str == L"EN") m_lang = Lang::EN;
}

const wchar_t* I18n::T(const char* key) const {
    auto it = m_strings.find(key);
    if (it == m_strings.end()) return L"???";
    return (m_lang == Lang::JA) ? it->second.ja : it->second.en;
}

void I18n::Init() {
    // Tray menu
    m_strings["tray.show"]     = { L"\x8868\x793A",   L"Show" };         // Show
    m_strings["tray.settings"] = { L"\x8A2D\x5B9A",   L"Settings" };     // Settings
    m_strings["tray.exit"]     = { L"\x7D42\x4E86",   L"Exit" };         // Exit

    // Tooltips
    m_strings["tip.pin_on"]   = { L"\x524D\x9762\x7DAD\x6301: ON\n\x30AF\x30EA\x30C3\x30AF\x3067\x5207\x66FF",
                                  L"Always on Top: ON\nClick to toggle" };
    m_strings["tip.pin_off"]  = { L"\x524D\x9762\x7DAD\x6301: OFF\n\x30AF\x30EA\x30C3\x30AF\x3067\x5207\x66FF",
                                  L"Always on Top: OFF\nClick to toggle" };
    m_strings["tip.settings"] = { L"\x8A2D\x5B9A", L"Settings" };
    m_strings["tip.help"]     = { L"\x30D8\x30EB\x30D7", L"Help" };

    // Context menu
    m_strings["menu.delete"] = { L"\x524A\x9664", L"Delete" };  // Delete
    m_strings["menu.edit"]   = { L"\x7DE8\x96C6", L"Edit" };    // Edit
    m_strings["menu.add"]    = { L"\x65B0\x898F\x8FFD\x52A0", L"Add" };  // Add
    m_strings["edit.title"]  = { L"\x30A2\x30D7\x30EA\x306E\x7DE8\x96C6", L"Edit App" };
    m_strings["add.title"]   = { L"\x30A2\x30D7\x30EA\x306E\x8FFD\x52A0", L"Add App" };
    m_strings["edit.name"]   = { L"\x540D\x524D:", L"Name:" };   // Name
    m_strings["edit.path"]   = { L"\x30D1\x30B9:", L"Path:" };   // Path

    // Main window
    m_strings["main.no_match"] = { L"\x4E00\x81F4\x3059\x308B\x30A2\x30D7\x30EA\x304C\x3042\x308A\x307E\x305B\x3093",
                                   L"No matching apps" };  // No matching apps

    // Error messages
    m_strings["err.no_config"] = { L"config.ini \x304C\x898B\x3064\x304B\x3089\x306A\x3044\x304B\x3001[Apps] \x30BB\x30AF\x30B7\x30E7\x30F3\x304C\x7A7A\x3067\x3059\x3002\n"
                                   L"EXE \x3068\x540C\x3058\x30D5\x30A9\x30EB\x30C0\x306B config.ini \x3092\x914D\x7F6E\x3057\x3066\x304F\x3060\x3055\x3044\x3002",
                                   L"config.ini not found or [Apps] section is empty.\n"
                                   L"Place config.ini in the same folder as the EXE." };
    m_strings["err.window"]    = { L"\x30A6\x30A3\x30F3\x30C9\x30A6\x306E\x4F5C\x6210\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002",
                                   L"Failed to create window." };
    m_strings["err.hotkey"]    = { L"\x30DB\x30C3\x30C8\x30AD\x30FC\x306E\x767B\x9332\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002\n"
                                   L"\x4ED6\x306E\x30A2\x30D7\x30EA\x304C\x4F7F\x7528\x4E2D\x306E\x53EF\x80FD\x6027\x304C\x3042\x308A\x307E\x3059\x3002",
                                   L"Failed to register hotkey.\n"
                                   L"Another application may be using it." };
    m_strings["err.gdiplus"]   = { L"GDI+ \x306E\x521D\x671F\x5316\x306B\x5931\x6557\x3057\x307E\x3057\x305F\x3002",
                                   L"Failed to initialize GDI+." };

    // Settings dialog
    m_strings["settings.title"]     = { L"\x8A2D\x5B9A - FuWinLauncher", L"Settings - FuWinLauncher" };
    m_strings["settings.app_list"]  = { L"\x30A2\x30D7\x30EA\x4E00\x89A7:",  L"App List:" };  // App list
    m_strings["settings.add"]       = { L"\x8FFD\x52A0",    L"Add" };      // Add
    m_strings["settings.delete"]    = { L"\x524A\x9664",    L"Delete" };   // Delete
    m_strings["settings.hotkey"]    = { L"\x30DB\x30C3\x30C8\x30AD\x30FC:", L"HotKey:" }; // HotKey
    m_strings["settings.opacity"]   = { L"\x900F\x660E\x5EA6:",  L"Opacity:" };  // Opacity
    m_strings["settings.maxheight"] = { L"\x6700\x5927\x9AD8\x3055:", L"MaxHeight:" }; // Max height
    m_strings["settings.autoresize"] = { L"\x30A6\x30A3\x30F3\x30C9\x30A6\x9AD8\x3055\x3092\x81EA\x52D5\x8ABF\x6574",
                                         L"Auto-resize window height" };
    m_strings["settings.topmost"]   = { L"\x5E38\x306B\x6700\x524D\x9762\x306B\x8868\x793A", L"Always on Top" };
    m_strings["settings.showsettingsbtn"] = {
        L"\x8A2D\x5B9A\x30DC\x30BF\x30F3\x3092\x8868\x793A\xFF08\u203B\x975E\x8868\x793A\x5F8C\x306F\x30C8\x30EC\x30A4\x2192\x53F3\x30AF\x30EA\x30C3\x30AF\x3067\x8868\x793A\x53EF\xFF09",
        L"Show settings button (*Hidden: use Tray -> Right-click)" };
    m_strings["settings.showhelpbtn"]    = {
        L"\x30D8\x30EB\x30D7\x30DC\x30BF\x30F3\x3092\x8868\x793A\xFF08\u203B\x975E\x8868\x793A\x5F8C\x306F\x30C8\x30EC\x30A4\x2192\x53F3\x30AF\x30EA\x30C3\x30AF\x3067\x8868\x793A\x53EF\xFF09",
        L"Show help button (*Hidden: use Tray -> Right-click)" };
    m_strings["settings.showtoolbar"]  = { L"\x30C4\x30FC\x30EB\x30D0\x30FC\x3092\x8868\x793A",
                                           L"Show toolbar" };
    m_strings["settings.hideonlaunch"] = { L"\x8D77\x52D5\x5F8C\x306B\x30A6\x30A3\x30F3\x30C9\x30A6\x3092\x96A0\x3059",
                                           L"Hide window after launch" };
    // Theme section in settings
    m_strings["settings.theme"]          = { L"\x25A0 \x30C6\x30FC\x30DE", L"\x25A0 Theme" };
    m_strings["settings.titletext"]      = { L"\x30BF\x30A4\x30C8\x30EB:", L"Title:" };
    m_strings["settings.titlebarcolor"]  = { L"\x30BF\x30A4\x30C8\x30EB\x30D0\x30FC\x8272:", L"Title bar:" };
    m_strings["settings.titletextcolor"] = { L"\x30BF\x30A4\x30C8\x30EB\x6587\x5B57:", L"Title text:" };
    m_strings["settings.bgcolor"]        = { L"\x80CC\x666F\x8272:", L"Background:" };
    m_strings["settings.textcolor"]      = { L"\x6587\x5B57\x8272:", L"Text color:" };
    m_strings["settings.selectcolor"]    = { L"\x9078\x629E\x8272:", L"Select color:" };
    m_strings["settings.searchbg"]       = { L"\x691C\x7D22\x80CC\x666F:", L"Search BG:" };
    m_strings["settings.searchtext"]     = { L"\x691C\x7D22\x6587\x5B57:", L"Search text:" };
    m_strings["settings.bgimage"]        = { L"\x80CC\x666F\x753B\x50CF:", L"BG image:" };
    m_strings["settings.bgimagemode"]  = { L"\x753B\x50CF\x8868\x793A:", L"Image mode:" };
    m_strings["settings.mode.center"]  = { L"\x4E2D\x592E", L"Center" };
    m_strings["settings.mode.stretch"] = { L"\x30B9\x30C8\x30EC\x30C3\x30C1", L"Stretch" };
    m_strings["settings.mode.tile"]    = { L"\x7E70\x308A\x8FD4\x3057", L"Tile" };
    m_strings["settings.bgimagealpha"]   = { L"\x753B\x50CF\x900F\x660E\x5EA6:", L"Image alpha:" };
    m_strings["settings.customicon"]     = { L"\x30A2\x30A4\x30B3\x30F3:", L"Icon:" };
    m_strings["settings.language"]       = { L"\x8A00\x8A9E:", L"Language:" };
    m_strings["settings.hotkey"]         = { L"\x30DB\x30C3\x30C8\x30AD\x30FC:", L"Hotkey:" };
    m_strings["settings.skin"]           = { L"\x30B9\x30AD\x30F3:", L"Skin:" };
    m_strings["settings.skin_none"]      = { L"(\x306A\x3057)", L"(none)" };
    m_strings["settings.browse"]         = { L"...", L"..." };

    m_strings["settings.ok"]        = { L"OK", L"OK" };
    m_strings["settings.cancel"]    = { L"\x30AD\x30E3\x30F3\x30BB\x30EB", L"Cancel" };  // Cancel

    // Help
    m_strings["tray.help"] = { L"\x30D8\x30EB\x30D7", L"Help" };  // Help
    m_strings["help.title"] = { L"FuWinLauncher - \x30D8\x30EB\x30D7", L"FuWinLauncher - Help" };
    m_strings["help.support"] = {
        L"\x2764 <a href=\"https://ko-fi.com/fufoo\">Ko-fi \x3067\x5FDC\x63F4\x3059\x308B</a>",
        L"\x2764 <a href=\"https://ko-fi.com/fufoo\">Support on Ko-fi</a>"
    };
    m_strings["help.text"] = {
        // Japanese
        L"FuWinLauncher - \x30E9\x30F3\x30C1\x30E3\x30FC\x30A2\x30D7\x30EA\r\n"
        L"\r\n"
        L"\x25A0 \x30B7\x30E7\x30FC\x30C8\x30AB\x30C3\x30C8\x30AD\x30FC\r\n"
        L"  Alt+Space  \x8868\x793A / \x975E\x8868\x793A\x5207\x66FF\r\n"
        L"  \x2191 / \x2193        \x9078\x629E\x79FB\x52D5\r\n"
        L"  Enter       \x30A2\x30D7\x30EA\x8D77\x52D5\r\n"
        L"  Esc          \x30A6\x30A3\x30F3\x30C9\x30A6\x3092\x96A0\x3059\r\n"
        L"\r\n"
        L"\x25A0 \x30A2\x30D7\x30EA\x7BA1\x7406\r\n"
        L"  \x30FB \x53F3\x30AF\x30EA\x30C3\x30AF \x2192 \x7DE8\x96C6\x30FB\x524A\x9664\x30FB\x65B0\x898F\x8FFD\x52A0\r\n"
        L"  \x30FB \x30C9\x30E9\x30C3\x30B0\x0026\x30C9\x30ED\x30C3\x30D7\x3067\x30A2\x30D7\x30EA\x8FFD\x52A0\r\n"
        L"  \x30FB \x30EA\x30B9\x30C8\x5185\x30C9\x30E9\x30C3\x30B0\x3067\x4E26\x3079\x66FF\x3048\r\n"
        L"  \x30FB .exe / .lnk / .bat / .cmd / .ps1 / .com / .vbs / .wsf / .msi / URL \x306B\x5BFE\x5FDC\r\n"
        L"\r\n"
        L"\x25A0 \x305D\x306E\x4ED6\r\n"
        L"  \x30FB \x6587\x5B57\x5165\x529B\x3067\x30A2\x30D7\x30EA\x3092\x7D5E\x308A\x8FBC\x307F\r\n"
        L"  \x30FB \x30BF\x30B9\x30AF\x30C8\x30EC\x30A4\x53F3\x30AF\x30EA\x30C3\x30AF\x3067\x8A2D\x5B9A\x30FB\x30D8\x30EB\x30D7\r\n"
        L"  \x30FB \x2699 \x8A2D\x5B9A\x3067\x30C6\x30FC\x30DE\x30FB\x8272\x30FB\x80CC\x666F\x753B\x50CF\x306E\x30AB\x30B9\x30BF\x30DE\x30A4\x30BA\r\n"
        L"  \x30FB \x65E5\x672C\x8A9E / English \x5BFE\x5FDC\r\n"
        L"\r\n"
        L"GitHub: https://github.com/fu-foo/FuWinLauncher\r\n",
        // English
        L"FuWinLauncher - Launcher App\r\n"
        L"\r\n"
        L"\x25A0 Shortcut Keys\r\n"
        L"  Alt+Space  Show / Hide toggle\r\n"
        L"  \x2191 / \x2193        Move selection\r\n"
        L"  Enter       Launch app\r\n"
        L"  Esc          Hide window\r\n"
        L"\r\n"
        L"\x25A0 App Management\r\n"
        L"  \x2022 Right-click \x2192 Edit / Delete / Add new\r\n"
        L"  \x2022 Drag & drop files to add apps\r\n"
        L"  \x2022 Drag items in list to reorder\r\n"
        L"  \x2022 Supports .exe / .lnk / .bat / .cmd / .ps1 / .com / .vbs / .wsf / .msi / URLs\r\n"
        L"\r\n"
        L"\x25A0 Other\r\n"
        L"  \x2022 Type to filter apps\r\n"
        L"  \x2022 Right-click tray for Settings / Help\r\n"
        L"  \x2022 \x2699 Customize theme, colors, background image\r\n"
        L"  \x2022 Japanese / English supported\r\n"
        L"\r\n"
        L"GitHub: https://github.com/fu-foo/FuWinLauncher\r\n"
    };

    // File dialog
    m_strings["filedialog.exe"]  = { L"\x5B9F\x884C\x30D5\x30A1\x30A4\x30EB (*.exe;*.com)", L"Executables (*.exe;*.com)" };
    m_strings["filedialog.lnk"]  = { L"\x30B7\x30E7\x30FC\x30C8\x30AB\x30C3\x30C8 (*.lnk)", L"Shortcuts (*.lnk)" };
    m_strings["filedialog.bat"]  = { L"\x30D0\x30C3\x30C1\x30D5\x30A1\x30A4\x30EB (*.bat;*.cmd)", L"Batch Files (*.bat;*.cmd)" };
    m_strings["filedialog.script"] = { L"\x30B9\x30AF\x30EA\x30D7\x30C8 (*.ps1;*.vbs;*.wsf)", L"Scripts (*.ps1;*.vbs;*.wsf)" };
    m_strings["filedialog.msi"]  = { L"\x30A4\x30F3\x30B9\x30C8\x30FC\x30E9 (*.msi)", L"Installers (*.msi)" };
    m_strings["filedialog.all"]  = { L"\x3059\x3079\x3066\x306E\x30D5\x30A1\x30A4\x30EB (*.*)", L"All Files (*.*)" };
}
