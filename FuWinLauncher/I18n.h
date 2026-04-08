#pragma once

#include <windows.h>
#include <string>
#include <unordered_map>

enum class Lang { JA, EN };

class I18n {
public:
    static I18n& Get();

    void SetLang(Lang lang) { m_lang = lang; }
    void SetLangFromString(const std::wstring& str);
    void DetectFromOS();
    Lang GetLang() const { return m_lang; }

    const wchar_t* T(const char* key) const;

private:
    I18n();
    void Init();

    Lang m_lang = Lang::JA;

    struct Entry {
        const wchar_t* ja;
        const wchar_t* en;
    };
    std::unordered_map<std::string, Entry> m_strings;
};
