# FuWinLauncher

A lightweight, customizable application launcher for Windows.

![Windows](https://img.shields.io/badge/platform-Windows-blue)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue)
![License](https://img.shields.io/badge/license-Apache%202.0-green)

## Features

- **Quick Launch** - Launch apps with `Alt+Space`, search by typing
- **App Management** - Add, edit, delete, reorder apps via right-click or drag & drop
- **Customizable Theme** - Colors, background image, opacity, custom icon
- **System Tray** - Minimizes to tray, right-click for settings/help
- **i18n** - Japanese / English support (auto-detects OS language)
- **Portable** - Single EXE + config.ini, no installer needed
- **Supports** - `.exe`, `.lnk` shortcuts, URLs

## Screenshot

```
┌─────────────────────────────┐
│ [Search...] [📌][⚙][ℹ]     │
│─────────────────────────────│
│ 🗒 Notepad                  │
│ 📁 Explorer                 │
│ 🌐 Chrome                   │
│ ...                         │
└─────────────────────────────┘
```

## Architecture

FuWinLauncher is built entirely with the Win32 API and C++ standard library — no frameworks, no .NET, no external dependencies. The result is a single standalone `.exe` that runs on any Windows machine without installing runtimes or libraries. Just download and run.

## Getting Started

1. Download `FuWinLauncher.exe` from [Releases](../../releases)
2. Place it anywhere and run
3. `config.ini` is auto-created on first launch
4. Press `Alt+Space` to show/hide the launcher

## Auto-start on Login

FuWinLauncher does not include an auto-start feature. To launch it automatically when you log in to Windows:

1. Press `Win + R`, type `shell:startup`, and press Enter
2. The Startup folder opens
3. Create a shortcut to `FuWinLauncher.exe` in that folder

That's it — it will start automatically on next login.

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Alt+Space` | Show / Hide |
| `↑` / `↓` | Move selection |
| `Enter` | Launch selected app |
| `Esc` | Hide window |
| Type text | Filter apps |

## App Management

- **Drag & drop** `.exe` / `.lnk` files onto the window to add
- **Right-click** an app → Edit / Delete
- **Right-click** empty area → Add new
- **Drag** items in the list to reorder

## Configuration

`config.ini` is created next to the EXE. Edit it directly or use the settings dialog (⚙ button / tray right-click → Settings).

```ini
[Apps]
Notepad=C:\Windows\notepad.exe
Explorer=C:\Windows\explorer.exe
Chrome=C:\Program Files\Google\Chrome\Application\chrome.exe

[Settings]
Opacity=200
MaxHeight=600
Topmost=on
HideOnLaunch=off
ShowSettingsButton=on
ShowHelpButton=on
Language=ja

[Theme]
TitleText=FuWinLauncher
TitleBarColor=#1E1E28
TitleTextColor=#FFFFFF
BgColor=#1E1E28
TextColor=#F0F0F0
SelectColor=#3C5078
SearchBgColor=#2D2D3C
SearchTextColor=#F0F0F0
BgImage=
BgImageAlpha=40
BgImageMode=center
CustomIcon=
```

### Settings Reference

| Key | Description | Default |
|-----|-------------|---------|
| `Opacity` | Window opacity (50-255) | `200` |
| `MaxHeight` | Max window height in px | `600` |
| `Topmost` | Always on top (`on`/`off`) | `on` |
| `HideOnLaunch` | Hide after launching app | `off` |
| `ShowSettingsButton` | Show ⚙ button | `on` |
| `ShowHelpButton` | Show ℹ button | `on` |
| `Language` | `ja` or `en` (empty = auto) | *(auto)* |

### Theme Reference

| Key | Description | Default |
|-----|-------------|---------|
| `TitleText` | Window title | `FuWinLauncher` |
| `TitleBarColor` | Title bar color | `#1E1E28` |
| `TitleTextColor` | Title bar text color | `#FFFFFF` |
| `BgColor` | Background color | `#1E1E28` |
| `TextColor` | App name text color | `#F0F0F0` |
| `SelectColor` | Selected item color | `#3C5078` |
| `SearchBgColor` | Search box background | `#2D2D3C` |
| `SearchTextColor` | Search box text | `#F0F0F0` |
| `BgImage` | Background image path (png/jpg/bmp) | *(empty)* |
| `BgImageAlpha` | Image opacity (0-255) | `40` |
| `BgImageMode` | `center` / `stretch` / `tile` | `center` |
| `CustomIcon` | Custom .ico file path | *(empty)* |

## Build

### Requirements

- Visual Studio 2022 (v143 toolset)
- Windows SDK 10.0

### Build from command line

```bash
# x64 Release
msbuild FuWinLauncher.sln /p:Configuration=Release /p:Platform=x64

# x86 Release
msbuild FuWinLauncher.sln /p:Configuration=Release /p:Platform=Win32
```

### Output

| Configuration | Path |
|---------------|------|
| x64 Release | `x64/Release/FuWinLauncher.exe` |
| x86 Release | `Release/FuWinLauncher.exe` |

## Support

If you find this project useful, consider supporting it:

[![GitHub Sponsors](https://img.shields.io/badge/Sponsor-GitHub-ea4aaa?logo=github)](https://github.com/sponsors/fu-foo)
[![Ko-fi](https://img.shields.io/badge/Support-Ko--fi-FF5E5B?logo=kofi)](https://ko-fi.com/fufoo)

## License

Apache License 2.0 - See [LICENSE](LICENSE) for details.

---

# FuWinLauncher (日本語)

Windows 向けの軽量カスタマイズ可能なアプリケーションランチャーです。

## 設計

Win32 API と C++ 標準ライブラリのみで構築しています。.NET やフレームワーク、外部ライブラリに一切依存しないため、EXE 単体でどの Windows 環境でもそのまま動作します。ランタイムのインストールは不要です。

## 特徴

- `Alt+Space` でランチャーを表示、文字入力でアプリを絞り込み
- 右クリックやドラッグ＆ドロップでアプリの追加・編集・削除・並べ替え
- テーマ（色・背景画像・透明度・アイコン）のカスタマイズ
- タスクトレイ常駐、右クリックで設定・ヘルプ
- 日本語 / 英語対応（OS言語自動判定）
- EXE単体 + config.ini のポータブル動作
- `.exe`、`.lnk` ショートカット、URL に対応

## 使い方

1. [Releases](../../releases) から `FuWinLauncher.exe` をダウンロード
2. 任意の場所に置いて実行（初回起動時に `config.ini` が自動生成されます）
3. `Alt+Space` でランチャーの表示/非表示を切り替え

## ログイン時に自動起動

自動起動機能は内蔵していません。Windows ログイン時に自動で起動するには:

1. `Win + R` を押して `shell:startup` と入力し Enter
2. スタートアップフォルダが開きます
3. `FuWinLauncher.exe` のショートカットをそのフォルダに作成

次回ログインから自動的に起動します。

## キーボード操作

| キー | 動作 |
|------|------|
| `Alt+Space` | 表示 / 非表示 |
| `↑` / `↓` | 選択移動 |
| `Enter` | アプリ起動 |
| `Esc` | ウィンドウを隠す |
| 文字入力 | アプリ絞り込み |

設定項目の詳細は上記英語セクションの Settings Reference / Theme Reference を参照してください。

## サポート

このプロジェクトが役に立ったら、ぜひ応援をお願いします:

[![GitHub Sponsors](https://img.shields.io/badge/Sponsor-GitHub-ea4aaa?logo=github)](https://github.com/sponsors/fu-foo)
[![Ko-fi](https://img.shields.io/badge/Support-Ko--fi-FF5E5B?logo=kofi)](https://ko-fi.com/fufoo)
