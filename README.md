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
- **Supports** - `.exe`, `.lnk`, `.bat`, `.cmd`, `.ps1`, `.com`, `.vbs`, `.wsf`, `.msi`, URLs

## Screenshot

<img width="386" height="192" alt="FuWinLauncher main window" src="https://github.com/user-attachments/assets/680adaad-78c9-4e2d-9e29-11303e88562e" />


## Architecture

FuWinLauncher is built with the Win32 API and C++ standard library — no frameworks, no .NET, no external dependencies. The C runtime is statically linked, so the result is a single `.exe` that runs without installing runtimes or libraries. It is tested on Windows 11; Windows 10 is expected to work but is untested.

## Getting Started

### Scoop (recommended)

```powershell
scoop bucket add fu-foo https://github.com/fu-foo/scoop-bucket
scoop install fuwinlauncher
```

- Start it from the Start menu shortcut, or run `FuWinLauncher`.
- `config.ini` and `skins\` are kept in `~\scoop\persist\fuwinlauncher\`, so they survive updates. Wherever this README says "next to the EXE", use this folder instead.
- Update with `scoop update fuwinlauncher`. Uninstall with `scoop uninstall fuwinlauncher` (add `-p` to also delete your settings).
- Scoop verifies the download hash.

### Manual download

1. Download the EXE from [Releases](https://github.com/fu-foo/FuWinLauncher/releases):
   - `FuWinLauncher-x64.exe` — 64-bit Windows (also use this on ARM64 Windows 11, where it runs under emulation)
   - `FuWinLauncher-x86.exe` — 32-bit Windows
2. Place it in a folder that only you can write to, for example `%LOCALAPPDATA%\FuWinLauncher` (see [Security Notes](#security-notes) for why).
   - `C:\Program Files` does not work: writing there needs administrator rights, so `config.ini`, which is created next to the EXE, cannot be saved.
3. Run it. `config.ini` is auto-created on first launch.
4. Press `Alt+Space` to show/hide the launcher.

Only one FuWinLauncher runs at a time. Running the EXE again just brings the running one to the front.

## Windows SmartScreen Warning

This warning appears only for EXEs downloaded manually with a browser. It does not appear with Scoop, because Scoop downloads are not marked as coming from the internet.

Since the EXE is not digitally signed, Windows SmartScreen may show a blue warning dialog the first time you run each downloaded file — so you will see it again after downloading a new version. This is normal for unsigned open-source software.

To proceed:
1. Click **"More info"**
2. Click **"Run anyway"**

Alternatively, right-click the EXE → **Properties** → check **Unblock** before running it.

If Smart App Control is enabled (Windows 11) or your organization hides the "Run anyway" button, unsigned EXEs are blocked and the steps above do not help.

## Exiting FuWinLauncher

Closing the window (× button or `Esc`) hides it to the system tray — FuWinLauncher keeps running. To fully exit:

- Right-click the tray icon → **Exit**

## Auto-start on Login

FuWinLauncher does not include an auto-start feature. To launch it automatically when you log in to Windows:

1. Press `Win + R`, type `shell:startup`, and press Enter (the Startup folder opens)
2. Create a shortcut to `FuWinLauncher.exe` in that folder

It will start automatically on next login.

If you installed with Scoop, set the shortcut target to `%USERPROFILE%\scoop\apps\fuwinlauncher\current\FuWinLauncher.exe`. A target containing a version number keeps starting the old version after an update.

## Keyboard Shortcuts

| Key | Action |
|-----|--------|
| `Alt+Space` / `Ctrl+Space` | Show / Hide |
| `↑` / `↓` | Move selection |
| `Enter` | Launch selected app |
| `Esc` | Hide window |
| Type text | Filter apps (case-insensitive, matches anywhere in the name) |

The show/hide hotkey can be switched between `Alt+Space` (default) and `Ctrl+Space` from the Settings dialog.

- `Alt+Space` is also the Windows window-menu shortcut and the default key of PowerToys Run. In an RDP session, the Windows on the connecting side may handle the key so that it never reaches the remote machine.
- `Ctrl+Space` works in RDP. While FuWinLauncher is running, though, other apps no longer receive that key — for example, it stops triggering code completion in VS Code / Visual Studio.
- If another app already holds the hotkey, FuWinLauncher shows a "Failed to register hotkey" warning at startup and keeps running. To recover:
  1. Click the tray icon to open the window
  2. Switch the hotkey in Settings, or close the other app and restart FuWinLauncher

## App Management

- **Drag & drop** files with the extensions listed in [Features](#features) onto the window to add
- **Right-click** an app → Edit / Delete
- **Right-click** empty area → Add new (use this for URLs and other file types)
- **Drag** items in the list to reorder

## Security Notes

- FuWinLauncher runs whatever paths are listed in `config.ini`, and `config.ini` sits next to the EXE. If other users can write to that folder, they can make you launch any program.
- `.ps1` files are launched with `powershell.exe -ExecutionPolicy Bypass`, which bypasses the locally configured execution policy. Only register scripts you trust.
- A `.ps1` may still not run in two cases: when the execution policy is enforced by Group Policy (Group Policy takes precedence over `-ExecutionPolicy`), and when AppLocker / WDAC restricts scripts.
- Each release includes `SHA256SUMS.txt` (from v1.2.3). To verify a manual download, check that the output of this command matches the value in that file:

  ```powershell
  Get-FileHash .\FuWinLauncher-x64.exe -Algorithm SHA256
  ```

## Configuration

`config.ini` is created next to the EXE. Edit it directly or use the settings dialog (⚙ button / tray right-click → Settings). Restart FuWinLauncher after editing the file by hand.

Save the file as **UTF-8** (with or without BOM). Other encodings such as Shift_JIS garble non-ASCII app names.

```ini
[Apps]
Notepad=C:\Windows\notepad.exe
Explorer=C:\Windows\explorer.exe
Chrome=C:\Program Files\Google\Chrome\Application\chrome.exe
Downloads=%USERPROFILE%\Downloads
GitHub=https://github.com/

[Settings]
HotKey=Alt+Space
Opacity=200
MaxHeight=600
AutoResize=on
Topmost=on
HideOnLaunch=off
ShowSettingsButton=on
ShowHelpButton=on
Language=ja
Skin=

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

### Apps Reference

Each line is `Name=Path`. Apps are listed in the order they appear in the file.

- **Path** can be a file, a folder, or a URL. It is opened the same way as double-clicking it in Explorer. Environment variables such as `%USERPROFILE%` are expanded.
- **Arguments are not supported** — the whole value is treated as the path. To pass arguments or set a working folder, register a `.lnk` shortcut or a `.bat` file instead.
- **Names** cannot contain `=` and cannot start with `;`, `#` or `[`. The same name may be used more than once.
- Lines starting with `;` or `#` are comments. Comments are removed when FuWinLauncher saves the file (Settings dialog, or adding / editing / deleting an app).
- `.vbs` / `.wsf` rely on VBScript, which Microsoft is phasing out of Windows, so they may stop working in future Windows versions.

### Settings Reference

| Key | Description | Default |
|-----|-------------|---------|
| `HotKey` | Show/hide hotkey. Modifiers `Ctrl` / `Alt` / `Shift` / `Win` joined with `+`, then `Space`, `Return`, `Tab`, `Esc`, `A`-`Z` or `0`-`9`. The Settings dialog only offers `Alt+Space` and `Ctrl+Space` and resets other values when saved | `Alt+Space` |
| `Opacity` | Window opacity (50-255) | `200` |
| `MaxHeight` | Max window height in px (200-2000). Ignored while `AutoResize` is on | `600` |
| `AutoResize` | Auto-fit height to app count (`on`/`off`) | `on` |
| `Topmost` | Always on top (`on`/`off`) | `on` |
| `HideOnLaunch` | Hide the window after an app launches successfully (`on`/`off`) | `off` |
| `ShowSettingsButton` | Show ⚙ button | `on` |
| `ShowHelpButton` | Show ℹ button | `on` |
| `Language` | `ja` or `en` (empty = auto) | *(auto)* |
| `Skin` | Skin folder name under `skins/` (empty = none) | *(empty)* |

### Theme Reference

| Key | Description | Default |
|-----|-------------|---------|
| `TitleText` | Window title | `FuWinLauncher` |
| `TitleBarColor` | Title bar color (Windows 11 only) | `#1E1E28` |
| `TitleTextColor` | Title bar text color (Windows 11 only) | `#FFFFFF` |
| `BgColor` | Background color | `#1E1E28` |
| `TextColor` | App name text color | `#F0F0F0` |
| `SelectColor` | Selected item color | `#3C5078` |
| `SearchBgColor` | Search box background | `#2D2D3C` |
| `SearchTextColor` | Search box text | `#F0F0F0` |
| `BgImage` | Background image path (png/jpg/bmp) | *(empty)* |
| `BgImageAlpha` | Image opacity (0-255) | `40` |
| `BgImageMode` | `center` / `stretch` / `tile` | `center` |
| `CustomIcon` | Custom .ico file path | *(empty)* |

## Skins

You can override the theme with file-based skins. Place a `skins/` folder next to the EXE and create a subfolder for each skin:

```
FuWinLauncher.exe
config.ini
skins/
├── dark/
│   ├── theme.ini
│   └── background.png
└── retro/
    ├── theme.ini
    └── background.png
```

Each skin folder must contain `theme.ini` with a `[Theme]` section. The format is the same as `[Theme]` in `config.ini`. Image paths (`BgImage`, `CustomIcon`) must be relative to the skin folder.

Example `skins/dark/theme.ini`:

```ini
[Theme]
TitleText=My Launcher
TitleBarColor=#0D1B2A
BgColor=#1B263B
TextColor=#E0E1DD
SelectColor=#415A77
BgImage=background.png
BgImageAlpha=60
BgImageMode=center
```

Select a skin from **Settings → Theme → Skin**. Choose `(none)` to fall back to the `[Theme]` section in `config.ini`.

> No sample skins are bundled.

## Build

### Requirements

- Visual Studio 2022 (v143 toolset)
- Windows SDK 10.0
- Git (optional) — the version shown in Help and in the EXE properties is taken from the latest `v*` tag at build time. Without Git, or when building from a source zip, it shows `dev`.

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

Win32 API と C++ 標準ライブラリのみで構築しています。.NET やフレームワーク、外部ライブラリに依存せず、C ランタイムも静的リンクしているため、ランタイムをインストールしなくても EXE 単体で動作します。動作確認は Windows 11 で行っています。Windows 10 でも動く作りですが、未確認です。

## 特徴

- `Alt+Space` でランチャーを表示、文字入力でアプリを絞り込み
- 右クリックやドラッグ＆ドロップでアプリの追加・編集・削除・並べ替え
- テーマ（色・背景画像・透明度・アイコン）のカスタマイズ
- タスクトレイ常駐、右クリックで設定・ヘルプ
- 日本語 / 英語対応（OS言語自動判定）
- EXE単体 + config.ini のポータブル動作
- `.exe`、`.lnk`、`.bat`、`.cmd`、`.ps1`、`.com`、`.vbs`、`.wsf`、`.msi`、URL に対応

## 使い方

### Scoop（おすすめ）

```powershell
scoop bucket add fu-foo https://github.com/fu-foo/scoop-bucket
scoop install fuwinlauncher
```

- スタートメニューのショートカット、または `FuWinLauncher` コマンドで起動します。
- `config.ini` と `skins\` は `~\scoop\persist\fuwinlauncher\` に置かれるので、アップデートしても消えません。この README で「EXE と同じ場所」と書いている箇所は、Scoop ではこのフォルダに読み替えてください。
- アップデートは `scoop update fuwinlauncher`、アンインストールは `scoop uninstall fuwinlauncher`（設定ごと消すなら `-p` を付ける）。
- Scoop がダウンロードのハッシュを検証します。

### 手動ダウンロード

1. [Releases](https://github.com/fu-foo/FuWinLauncher/releases) から EXE をダウンロード
   - `FuWinLauncher-x64.exe` — 64bit Windows 用（ARM64 版 Windows 11 でもこちら。エミュレーションで動作します）
   - `FuWinLauncher-x86.exe` — 32bit Windows 用
2. 自分だけが書き込めるフォルダ（例: `%LOCALAPPDATA%\FuWinLauncher`）に置く（理由は[セキュリティ上の注意](#セキュリティ上の注意)を参照）
   - `C:\Program Files` には置けません。管理者権限がないと書き込めないフォルダなので、EXE と同じ場所に作られる `config.ini` を保存できません。
3. 実行する（初回起動時に `config.ini` が自動生成されます）
4. `Alt+Space` でランチャーの表示/非表示を切り替え

FuWinLauncher は同時に 1 つしか動きません。EXE をもう一度実行すると、動作中のウィンドウが前面に出ます。

## Windows SmartScreen の警告

この警告が出るのは、ブラウザで手動ダウンロードした EXE だけです。Scoop で入れた場合は出ません。Scoop のダウンロードには「インターネットから入手した」という印が付かないためです。

EXE にデジタル署名がないため、ダウンロードしたファイルを初めて実行するときに Windows SmartScreen の青い警告画面が表示されることがあります。新しいバージョンをダウンロードすれば、また表示されます。署名のないオープンソースソフトウェアでは一般的な動作です。

実行するには:
1. **「詳細情報」** をクリック
2. **「実行」** をクリック

EXE を右クリック → **プロパティ** → **「許可する」** にチェックを入れてから実行する方法もあります。

Windows 11 のスマート アプリ コントロールが有効な環境や、組織のポリシーで「実行」ボタンが隠されている環境では、署名のない EXE はブロックされ、上の手順では実行できません。

## FuWinLauncher の終了

ウィンドウを閉じる（×ボタンや `Esc`）とタスクトレイに格納されます。FuWinLauncher は終了しません。完全に終了するには:

- タスクトレイアイコンを右クリック → **終了**

## ログイン時に自動起動

自動起動機能は内蔵していません。Windows ログイン時に自動で起動するには:

1. `Win + R` を押して `shell:startup` と入力し Enter（スタートアップフォルダが開きます）
2. そのフォルダに `FuWinLauncher.exe` のショートカットを作成

次回ログインから自動的に起動します。

Scoop で入れた場合は、ショートカットのリンク先を `%USERPROFILE%\scoop\apps\fuwinlauncher\current\FuWinLauncher.exe` にしてください。バージョン番号を含むパスをリンク先にすると、アップデート後も古いバージョンが起動し続けます。

## キーボード操作

| キー | 動作 |
|------|------|
| `Alt+Space` / `Ctrl+Space` | 表示 / 非表示 |
| `↑` / `↓` | 選択移動 |
| `Enter` | アプリ起動 |
| `Esc` | ウィンドウを隠す |
| 文字入力 | アプリ絞り込み（大文字小文字を区別しない部分一致） |

表示/非表示のホットキーは、設定画面で `Alt+Space`（既定）と `Ctrl+Space` を切り替えられます。

- `Alt+Space` は Windows 標準のウィンドウメニューのショートカットで、PowerToys Run の既定キーでもあります。RDP セッションでは、接続元の Windows がこのキーを処理してしまい、接続先に届かないことがあります。
- `Ctrl+Space` は RDP でも使えます。ただし FuWinLauncher の動作中は、このキーが他のアプリに届かなくなります。たとえば VS Code / Visual Studio では、このキーで入力補完を呼び出せなくなります。
- 他のアプリが先にホットキーを登録していると、起動時に「ホットキーの登録に失敗しました」という警告が出ます。FuWinLauncher 自体は動き続けるので、次の手順で対処してください。
  1. タスクトレイのアイコンをクリックしてウィンドウを開く
  2. 設定でホットキーを切り替える。または相手のアプリを終了して FuWinLauncher を起動し直す

## アプリ管理

- [特徴](#特徴)に挙げた拡張子のファイルをウィンドウに**ドラッグ＆ドロップ**して追加
- アプリを**右クリック** → 編集 / 削除
- 空いている場所を**右クリック** → 新規追加（URL やその他の種類のファイルはこちらから）
- リスト内で**ドラッグ**して並べ替え

## セキュリティ上の注意

- FuWinLauncher は `config.ini` に書かれたパスをそのまま実行します。`config.ini` は EXE と同じ場所にあるので、他のユーザーがそのフォルダに書き込めると、任意のプログラムを起動させられてしまいます。
- `.ps1` は `powershell.exe -ExecutionPolicy Bypass` で起動するため、ローカルで設定された実行ポリシーを迂回します。信頼できるスクリプトだけを登録してください。
- 次の 2 つの環境では `.ps1` が動かないことがあります。実行ポリシーがグループポリシーで強制されている環境（`-ExecutionPolicy` よりグループポリシーが優先されます）と、AppLocker / WDAC でスクリプトが制限されている環境です。
- 各リリースには `SHA256SUMS.txt` が付いています（v1.2.3 以降）。手動ダウンロードした EXE は、次のコマンドの結果が `SHA256SUMS.txt` の値と一致するかで検証できます。

  ```powershell
  Get-FileHash .\FuWinLauncher-x64.exe -Algorithm SHA256
  ```

## 設定ファイル

`config.ini` は EXE と同じ場所に作られます。直接編集するか、設定画面（⚙ ボタン / トレイ右クリック → 設定）で変更します。手で編集したあとは FuWinLauncher を起動し直してください。

ファイルは **UTF-8**（BOM の有無はどちらでも可）で保存してください。Shift_JIS などで保存すると日本語のアプリ名が文字化けします。

書式の例は、上の英語セクションの Configuration にあります。

### [Apps]

1 行が `名前=パス` で、ファイルに書いた順に表示されます。

- **パス**にはファイル、フォルダ、URL を書けます。エクスプローラーでダブルクリックしたときと同じ方法で開きます。`%USERPROFILE%` などの環境変数は展開されます。
- **引数は指定できません**。値全体がパスとして扱われます。引数や作業フォルダを指定したいときは、`.lnk` ショートカットか `.bat` を登録してください。
- **名前**には `=` を含められません。`;`、`#`、`[` で始めることもできません。同じ名前を複数登録することはできます。
- `;` または `#` で始まる行はコメントです。FuWinLauncher がファイルを保存すると（設定画面、アプリの追加・編集・削除）、コメントは消えます。
- `.vbs` / `.wsf` は VBScript に依存しています。Microsoft が Windows から VBScript を段階的に廃止しているため、将来の Windows では動かなくなる可能性があります。

### [Settings]

| キー | 説明 | 既定値 |
|------|------|--------|
| `HotKey` | 表示/非表示のホットキー。修飾キー `Ctrl` / `Alt` / `Shift` / `Win` を `+` でつなぎ、最後に `Space`、`Return`、`Tab`、`Esc`、`A`〜`Z`、`0`〜`9` のいずれかを書く。設定画面で選べるのは `Alt+Space` と `Ctrl+Space` だけで、保存するとそれ以外の値は上書きされる | `Alt+Space` |
| `Opacity` | ウィンドウの不透明度（50〜255） | `200` |
| `MaxHeight` | ウィンドウの最大高さ（px、200〜2000）。`AutoResize` が `on` の間は無視される | `600` |
| `AutoResize` | アプリ数に合わせて高さを自動調整（`on`/`off`） | `on` |
| `Topmost` | 常に最前面に表示（`on`/`off`） | `on` |
| `HideOnLaunch` | アプリの起動に成功したらウィンドウを隠す（`on`/`off`） | `off` |
| `ShowSettingsButton` | ⚙ ボタンを表示 | `on` |
| `ShowHelpButton` | ℹ ボタンを表示 | `on` |
| `Language` | `ja` または `en`（空なら自動判定） | *（自動）* |
| `Skin` | `skins/` 以下のスキンフォルダ名（空ならスキンなし） | *（空）* |

### [Theme]

色、背景画像、アイコンを指定します。キーの一覧は上の英語セクションの Theme Reference を参照してください。`TitleBarColor` と `TitleTextColor` が効くのは Windows 11 だけです。

## スキン

EXE と同じ場所に `skins/` フォルダを作り、サブフォルダごとにスキンを配置するとテーマを差し替えられます:

```
FuWinLauncher.exe
config.ini
skins/
├── dark/
│   ├── theme.ini
│   └── background.png
└── retro/
    ├── theme.ini
    └── background.png
```

各スキンフォルダには `theme.ini` を置きます。`[Theme]` セクションの書式は `config.ini` と同じです。`BgImage` や `CustomIcon` のパスは、スキンフォルダからの相対パスで書いてください。

設定画面の **■ テーマ** にある **スキン** から選択します。`(なし)` を選べば `config.ini` の `[Theme]` セクションが使われます。

> サンプルスキンは同梱していません。

## ビルド

Visual Studio 2022（v143 ツールセット）と Windows SDK 10.0 が必要です。

ヘルプと EXE のプロパティに出るバージョンは、ビルド時に最新の `v*` タグから取ります。Git がない環境や、ソース zip からのビルドでは `dev` と表示されます。

```bash
# x64 Release
msbuild FuWinLauncher.sln /p:Configuration=Release /p:Platform=x64

# x86 Release
msbuild FuWinLauncher.sln /p:Configuration=Release /p:Platform=Win32
```

出力先は x64 が `x64/Release/FuWinLauncher.exe`、x86 が `Release/FuWinLauncher.exe` です。

## サポート

このプロジェクトが役に立ったら、ぜひ応援をお願いします:

[![GitHub Sponsors](https://img.shields.io/badge/Sponsor-GitHub-ea4aaa?logo=github)](https://github.com/sponsors/fu-foo)
[![Ko-fi](https://img.shields.io/badge/Support-Ko--fi-FF5E5B?logo=kofi)](https://ko-fi.com/fufoo)

## ライセンス

Apache License 2.0 — 詳細は [LICENSE](LICENSE) を参照してください。
