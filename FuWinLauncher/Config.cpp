#include "Config.h"
#include "I18n.h"
#include <algorithm>
#include <fstream>
#include <sstream>

static int SafeStoi(const std::string& s, int def = 0) {
    try { return std::stoi(s); }
    catch (...) { return def; }
}

// Convert UTF-8 string to std::wstring
static std::wstring Utf8ToWide(const std::string& utf8) {
    if (utf8.empty()) return L"";
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(),
                                  static_cast<int>(utf8.size()), nullptr, 0);
    if (len == 0) return L"";
    std::wstring wide(len, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(),
                        static_cast<int>(utf8.size()), wide.data(), len);
    return wide;
}

// Strip leading/trailing whitespace and BOM
static std::string Trim(const std::string& s) {
    size_t start = 0;
    // Skip UTF-8 BOM
    if (s.size() >= 3 &&
        static_cast<unsigned char>(s[0]) == 0xEF &&
        static_cast<unsigned char>(s[1]) == 0xBB &&
        static_cast<unsigned char>(s[2]) == 0xBF) {
        start = 3;
    }
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r'))
        start++;
    size_t end = s.size();
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r'))
        end--;
    return s.substr(start, end - start);
}

bool Config::Load(const std::wstring& iniPath) {
    m_iniPath = iniPath;

    // Read as UTF-8
    std::ifstream file(iniPath);
    if (!file.is_open()) return false;

    m_apps.clear();
    std::string currentSection;
    std::string line;

    while (std::getline(file, line)) {
        std::string trimmed = Trim(line);
        if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#')
            continue;

        // Section header
        if (trimmed.front() == '[' && trimmed.back() == ']') {
            currentSection = trimmed.substr(1, trimmed.size() - 2);
            continue;
        }

        // key=value
        size_t eq = trimmed.find('=');
        if (eq == std::string::npos) continue;

        std::string key = Trim(trimmed.substr(0, eq));
        std::string value = Trim(trimmed.substr(eq + 1));

        if (currentSection == "Apps") {
            AppEntry entry;
            entry.name = Utf8ToWide(key);
            entry.path = Utf8ToWide(value);
            m_apps.push_back(std::move(entry));
        } else if (currentSection == "Settings") {
            if (key == "Opacity") {
                int opacity = std::clamp(SafeStoi(value), 0, 255);
                m_opacity = static_cast<BYTE>(opacity);
            } else if (key == "HotKey") {
                ParseHotKey(Utf8ToWide(value));
            } else if (key == "MaxHeight") {
                m_maxHeight = std::clamp(SafeStoi(value), 200, 2000);
            } else if (key == "Topmost") {
                m_topmost = (value == "1" || value == "true" || value == "on" || value == "ON");
            } else if (key == "HideOnLaunch") {
                m_hideOnLaunch = (value == "1" || value == "true" || value == "on" || value == "ON");
            } else if (key == "ShowSettingsButton") {
                m_showSettingsButton = (value == "1" || value == "true" || value == "on" || value == "ON");
            } else if (key == "ShowHelpButton") {
                m_showHelpButton = (value == "1" || value == "true" || value == "on" || value == "ON");
            } else if (key == "Language") {
                m_language = Utf8ToWide(value);
            } else if (key == "Skin") {
                m_skin = Utf8ToWide(value);
            }
        } else if (currentSection == "Theme") {
            if (key == "TitleText") {
                m_theme.titleText = Utf8ToWide(value);
            } else if (key == "TitleBarColor") {
                m_theme.titleBarColor = ParseColor(value, m_theme.titleBarColor);
            } else if (key == "TitleTextColor") {
                m_theme.titleTextColor = ParseColor(value, m_theme.titleTextColor);
            } else if (key == "BgColor") {
                m_theme.bgColor = ParseColor(value, m_theme.bgColor);
            } else if (key == "TextColor") {
                m_theme.textColor = ParseColor(value, m_theme.textColor);
            } else if (key == "SelectColor") {
                m_theme.selectColor = ParseColor(value, m_theme.selectColor);
            } else if (key == "SearchBgColor") {
                m_theme.searchBgColor = ParseColor(value, m_theme.searchBgColor);
            } else if (key == "SearchTextColor") {
                m_theme.searchTextColor = ParseColor(value, m_theme.searchTextColor);
            } else if (key == "BgImage") {
                m_theme.bgImage = Utf8ToWide(value);
            } else if (key == "BgImageAlpha") {
                m_theme.bgImageAlpha = static_cast<BYTE>(std::clamp(SafeStoi(value), 0, 255));
            } else if (key == "BgImageMode") {
                if (value == "center") m_theme.bgImageMode = 0;
                else if (value == "stretch") m_theme.bgImageMode = 1;
                else if (value == "tile") m_theme.bgImageMode = 2;
            } else if (key == "CustomIcon") {
                m_theme.customIcon = Utf8ToWide(value);
            }
        }
    }

    return !m_apps.empty();
}

void Config::CreateDefault(const std::wstring& iniPath) {
    m_iniPath = iniPath;
    m_apps.clear();

    bool isJa = (I18n::Get().GetLang() == Lang::JA);

    AppEntry notepad;
    notepad.name = isJa ? L"\x30E1\x30E2\x5E33" : L"Notepad";  // メモ帳 / Notepad
    notepad.path = L"C:\\Windows\\notepad.exe";
    m_apps.push_back(std::move(notepad));

    AppEntry explorer;
    explorer.name = isJa ? L"\x30A8\x30AF\x30B9\x30D7\x30ED\x30FC\x30E9\x30FC" : L"Explorer";
    explorer.path = L"C:\\Windows\\explorer.exe";
    m_apps.push_back(std::move(explorer));

    Save();
}

// Convert std::wstring to UTF-8 std::string
static std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) return "";
    int len = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(),
                                  static_cast<int>(wide.size()), nullptr, 0, nullptr, nullptr);
    if (len == 0) return "";
    std::string utf8(len, '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(),
                        static_cast<int>(wide.size()), utf8.data(), len, nullptr, nullptr);
    return utf8;
}

bool Config::AppendApp(const std::wstring& name, const std::wstring& path) {
    // Skip if the same path already exists
    for (const auto& app : m_apps) {
        if (app.path == path) return false;
    }

    // Append to the [Apps] section of config.ini
    std::ofstream file(m_iniPath, std::ios::app);
    if (!file.is_open()) return false;

    std::string utf8Name = WideToUtf8(name);
    std::string utf8Path = WideToUtf8(path);
    file << utf8Name << "=" << utf8Path << "\n";
    file.close();

    // Also add to the in-memory list
    AppEntry entry;
    entry.name = name;
    entry.path = path;
    m_apps.push_back(std::move(entry));
    return true;
}

static bool IsAbsolutePath(const std::wstring& p) {
    if (p.size() >= 2 && p[1] == L':') return true;
    if (!p.empty() && (p[0] == L'\\' || p[0] == L'/')) return true;
    return false;
}

bool Config::LoadSkin(const std::wstring& skinName, const std::wstring& exeDir) {
    if (skinName.empty()) return false;

    std::wstring skinDir = exeDir + L"skins\\" + skinName + L"\\";
    std::wstring themePath = skinDir + L"theme.ini";

    std::ifstream file(themePath);
    if (!file.is_open()) return false;

    std::string currentSection;
    std::string line;
    while (std::getline(file, line)) {
        std::string trimmed = Trim(line);
        if (trimmed.empty() || trimmed[0] == ';' || trimmed[0] == '#') continue;

        if (trimmed.front() == '[' && trimmed.back() == ']') {
            currentSection = trimmed.substr(1, trimmed.size() - 2);
            continue;
        }

        size_t eq = trimmed.find('=');
        if (eq == std::string::npos) continue;

        std::string key = Trim(trimmed.substr(0, eq));
        std::string value = Trim(trimmed.substr(eq + 1));
        if (currentSection != "Theme") continue;

        if (key == "TitleText") {
            m_theme.titleText = Utf8ToWide(value);
        } else if (key == "TitleBarColor") {
            m_theme.titleBarColor = ParseColor(value, m_theme.titleBarColor);
        } else if (key == "TitleTextColor") {
            m_theme.titleTextColor = ParseColor(value, m_theme.titleTextColor);
        } else if (key == "BgColor") {
            m_theme.bgColor = ParseColor(value, m_theme.bgColor);
        } else if (key == "TextColor") {
            m_theme.textColor = ParseColor(value, m_theme.textColor);
        } else if (key == "SelectColor") {
            m_theme.selectColor = ParseColor(value, m_theme.selectColor);
        } else if (key == "SearchBgColor") {
            m_theme.searchBgColor = ParseColor(value, m_theme.searchBgColor);
        } else if (key == "SearchTextColor") {
            m_theme.searchTextColor = ParseColor(value, m_theme.searchTextColor);
        } else if (key == "BgImage") {
            std::wstring p = Utf8ToWide(value);
            m_theme.bgImage = IsAbsolutePath(p) ? p : (skinDir + p);
        } else if (key == "BgImageAlpha") {
            m_theme.bgImageAlpha = static_cast<BYTE>(std::clamp(SafeStoi(value), 0, 255));
        } else if (key == "BgImageMode") {
            if (value == "center") m_theme.bgImageMode = 0;
            else if (value == "stretch") m_theme.bgImageMode = 1;
            else if (value == "tile") m_theme.bgImageMode = 2;
        } else if (key == "CustomIcon") {
            std::wstring p = Utf8ToWide(value);
            m_theme.customIcon = IsAbsolutePath(p) ? p : (skinDir + p);
        }
    }
    return true;
}

void Config::SaveApps() {
    // Read the ini file and rewrite the [Apps] section with the current m_apps
    std::ifstream inFile(m_iniPath);
    std::vector<std::string> lines;
    std::string line;
    bool inApps = false;
    bool appsWritten = false;

    while (std::getline(inFile, line)) {
        std::string trimmed = Trim(line);

        if (!trimmed.empty() && trimmed.front() == '[' && trimmed.back() == ']') {
            if (inApps && !appsWritten) {
                for (const auto& app : m_apps) {
                    lines.push_back(WideToUtf8(app.name) + "=" + WideToUtf8(app.path));
                }
                appsWritten = true;
            }
            inApps = (trimmed == "[Apps]");
            lines.push_back(line);
            continue;
        }

        if (inApps) {
            if (!trimmed.empty() && trimmed[0] != ';' && trimmed[0] != '#' &&
                trimmed.find('=') != std::string::npos) {
                continue;
            }
        }
        lines.push_back(line);
    }
    inFile.close();

    // Handle case where [Apps] is the last section in the file
    if (inApps && !appsWritten) {
        for (const auto& app : m_apps) {
            lines.push_back(WideToUtf8(app.name) + "=" + WideToUtf8(app.path));
        }
    }

    std::ofstream outFile(m_iniPath, std::ios::trunc);
    for (const auto& l : lines) {
        outFile << l << "\n";
    }
}

void Config::Save() {
    std::ofstream file(m_iniPath, std::ios::trunc);
    if (!file.is_open()) return;

    // [Apps]
    file << "[Apps]\n";
    for (const auto& app : m_apps) {
        file << WideToUtf8(app.name) << "=" << WideToUtf8(app.path) << "\n";
    }
    file << "\n";

    // [Settings]
    file << "[Settings]\n";
    file << "HotKey=" << WideToUtf8(HotKeyToString(m_hotKeyMod, m_hotKeyVK)) << "\n";
    file << "Opacity=" << static_cast<int>(m_opacity) << "\n";
    file << "MaxHeight=" << m_maxHeight << "\n";
    file << "Topmost=" << (m_topmost ? "on" : "off") << "\n";
    file << "HideOnLaunch=" << (m_hideOnLaunch ? "on" : "off") << "\n";
    file << "ShowSettingsButton=" << (m_showSettingsButton ? "on" : "off") << "\n";
    file << "ShowHelpButton=" << (m_showHelpButton ? "on" : "off") << "\n";
    if (!m_language.empty()) {
        file << "Language=" << WideToUtf8(m_language) << "\n";
    }
    if (!m_skin.empty()) {
        file << "Skin=" << WideToUtf8(m_skin) << "\n";
    }
    file << "\n";

    // [Theme]
    file << "[Theme]\n";
    file << "TitleText=" << WideToUtf8(m_theme.titleText) << "\n";
    file << "TitleBarColor=" << ColorToString(m_theme.titleBarColor) << "\n";
    file << "TitleTextColor=" << ColorToString(m_theme.titleTextColor) << "\n";
    file << "BgColor=" << ColorToString(m_theme.bgColor) << "\n";
    file << "TextColor=" << ColorToString(m_theme.textColor) << "\n";
    file << "SelectColor=" << ColorToString(m_theme.selectColor) << "\n";
    file << "SearchBgColor=" << ColorToString(m_theme.searchBgColor) << "\n";
    file << "SearchTextColor=" << ColorToString(m_theme.searchTextColor) << "\n";
    if (!m_theme.bgImage.empty()) {
        file << "BgImage=" << WideToUtf8(m_theme.bgImage) << "\n";
        file << "BgImageAlpha=" << static_cast<int>(m_theme.bgImageAlpha) << "\n";
        const char* modes[] = { "center", "stretch", "tile" };
        file << "BgImageMode=" << modes[std::clamp(m_theme.bgImageMode, 0, 2)] << "\n";
    }
    if (!m_theme.customIcon.empty()) {
        file << "CustomIcon=" << WideToUtf8(m_theme.customIcon) << "\n";
    }
}

COLORREF Config::ParseColor(const std::string& str, COLORREF def) {
    if (str.empty()) return def;
    std::string s = str;
    if (s[0] == '#') s = s.substr(1);
    if (s.size() != 6) return def;
    unsigned int val = 0;
    for (char c : s) {
        val <<= 4;
        if (c >= '0' && c <= '9') val |= (c - '0');
        else if (c >= 'a' && c <= 'f') val |= (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') val |= (c - 'A' + 10);
        else return def;
    }
    // #RRGGBB -> COLORREF (0x00BBGGRR)
    BYTE r = (val >> 16) & 0xFF;
    BYTE g = (val >> 8) & 0xFF;
    BYTE b = val & 0xFF;
    return RGB(r, g, b);
}

std::string Config::ColorToString(COLORREF c) {
    char buf[8];
    snprintf(buf, sizeof(buf), "#%02X%02X%02X", GetRValue(c), GetGValue(c), GetBValue(c));
    return buf;
}

std::wstring Config::HotKeyToString(UINT mod, UINT vk) {
    std::wstring result;
    if (mod & MOD_CONTROL) result += L"Ctrl+";
    if (mod & MOD_ALT)     result += L"Alt+";
    if (mod & MOD_SHIFT)   result += L"Shift+";
    if (mod & MOD_WIN)     result += L"Win+";

    switch (vk) {
    case VK_SPACE:  result += L"Space"; break;
    case VK_RETURN: result += L"Return"; break;
    case VK_TAB:    result += L"Tab"; break;
    case VK_ESCAPE: result += L"Esc"; break;
    default:
        if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9')) {
            result += static_cast<wchar_t>(vk);
        } else {
            result += L"Space";
        }
        break;
    }
    return result;
}

void Config::ParseHotKey(const std::wstring& str) {
    m_hotKeyMod = 0;
    m_hotKeyVK = 0;

    std::wstring s = str;
    std::transform(s.begin(), s.end(), s.begin(), ::towupper);

    if (s.find(L"ALT") != std::wstring::npos)   m_hotKeyMod |= MOD_ALT;
    if (s.find(L"CTRL") != std::wstring::npos)   m_hotKeyMod |= MOD_CONTROL;
    if (s.find(L"SHIFT") != std::wstring::npos)  m_hotKeyMod |= MOD_SHIFT;
    if (s.find(L"WIN") != std::wstring::npos)     m_hotKeyMod |= MOD_WIN;

    size_t lastPlus = s.rfind(L'+');
    std::wstring key = (lastPlus != std::wstring::npos) ? s.substr(lastPlus + 1) : s;

    while (!key.empty() && key.front() == L' ') key.erase(key.begin());
    while (!key.empty() && key.back() == L' ')  key.pop_back();

    if (key == L"SPACE")       m_hotKeyVK = VK_SPACE;
    else if (key == L"RETURN") m_hotKeyVK = VK_RETURN;
    else if (key == L"TAB")    m_hotKeyVK = VK_TAB;
    else if (key == L"ESC")    m_hotKeyVK = VK_ESCAPE;
    else if (key.size() == 1 && key[0] >= L'A' && key[0] <= L'Z')
        m_hotKeyVK = static_cast<UINT>(key[0]);
    else if (key.size() == 1 && key[0] >= L'0' && key[0] <= L'9')
        m_hotKeyVK = static_cast<UINT>(key[0]);

    if (m_hotKeyVK == 0) {
        m_hotKeyMod = MOD_ALT;
        m_hotKeyVK = VK_SPACE;
    }
}
