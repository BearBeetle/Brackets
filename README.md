# Brackets

[日本語](#日本語) | [English](#english)

---

<a id="日本語"></a>

## 日本語

複数行テキストの各行先頭に、指定したインデント記号を付加して出力する Win32 デスクトップツールです。
Visual Studio 2022 / Win32 API（純正・Unicode）で実装しています。

### 概要

- 入力欄に複数行テキストを入力
- インデント記号欄に付加したい文字列を指定（既定値: `    >` ＝半角スペース4個+`>`）
- 「実行」ボタン押下で、各行の先頭にインデント記号を付加した結果を出力欄へ表示
- 実行時、インデント記号は `Brackets.ini` へ自動保存される

### ビルド方法

1. Visual Studio 2022 で `Brackets.sln`（または該当プロジェクト）を開く
2. 文字セットは **Unicode** を使用（既定設定のまま）
3. 構成 (Debug/Release) とプラットフォーム (x64/Win32) を選択
4. ビルド（`Ctrl+Shift+B`）を実行
5. 生成された `Brackets.exe` を実行

#### 使用ファイル

- `main.cpp`
- `resource.h`
- `Brackets.rc`
- `targetver.h`

MFC / ATL / C++/CLI / .NET 等には一切依存せず、純粋な Win32 API のみで構成されています。

### INI 仕様

- ファイル名: `Brackets.ini`
- 配置場所: `Brackets.exe` と同一フォルダ（カレントディレクトリに依存しない）
- セクション: `[Settings]`
- キー: `IndentString`

### 制限事項

- インデント記号は `GetPrivateProfileStringW` の仕様上、最大 1023 文字までを想定（実用上十分な長さ）
- 非常に大きなテキスト（数百万文字級）を扱う場合、`EDIT` コントロールの文字数制限（既定は約 64KB〜、`EM_LIMITTEXT` 未設定時は自動拡張）に依存する
- レイアウトはウィンドウサイズに応じて簡易的に再配置されるが、極端に小さいウィンドウサイズでは重なりが発生する可能性がある
- 日本語以外の言語環境での表示崩れは未検証
- 入力欄へクリップボードから貼り付けを行う際、貼り付け元テキストの改行コードが `LF` または `CR` 単体の場合、Win32 の `EDIT` コントロールの仕様上、画面上で改行として表示されないことがある
  - 「実行」ボタン押下時の変換処理（`ConvertText` / `SplitLines`）自体は `CRLF` / `LF` / `CR` いずれにも対応しているため、変換結果（出力欄）には影響しない
  - 回避策: 貼り付け後に対象行の末尾へ一度カーソルを置き、`Enter` キーを押し直すことで表示上も改行される

### ライセンス

[MIT License](LICENSE) のもとで公開しています。

---

<a id="english"></a>

## English

A Win32 desktop tool that prepends a specified indent string to the beginning of every line of multi-line text and outputs the result.
It is implemented with Visual Studio 2022 using the pure Win32 API (Unicode).

### Overview

- Enter multi-line text in the input box.
- Specify the string to prepend in the indent string box (default: `    >` = four half-width spaces followed by `>`).
- Press the "Run" button to display the result, with the indent string added to the start of each line, in the output box.
- On execution, the indent string is automatically saved to `Brackets.ini`.

### Build Instructions

1. Open `Brackets.sln` (or the relevant project) in Visual Studio 2022.
2. Use the **Unicode** character set (keep the default setting).
3. Select the configuration (Debug/Release) and platform (x64/Win32).
4. Build (`Ctrl+Shift+B`).
5. Run the generated `Brackets.exe`.

#### Source Files

- `main.cpp`
- `resource.h`
- `Brackets.rc`
- `targetver.h`

The project has no dependency on MFC, ATL, C++/CLI, .NET, or similar frameworks; it is built purely on the Win32 API.

### INI Specification

- File name: `Brackets.ini`
- Location: the same folder as `Brackets.exe` (independent of the current directory)
- Section: `[Settings]`
- Key: `IndentString`

### Limitations

- Due to the specification of `GetPrivateProfileStringW`, the indent string is expected to be at most 1023 characters (more than sufficient for practical use).
- When handling very large text (millions of characters), the tool is subject to the character limit of the `EDIT` control (roughly 64 KB by default; it expands automatically when `EM_LIMITTEXT` is not set).
- The layout is rearranged in a simple manner according to the window size, but elements may overlap if the window is made extremely small.
- Display issues in non-Japanese language environments have not been verified.
- When pasting from the clipboard into the input box, if the source text uses a lone `LF` or `CR` as the line break, the Win32 `EDIT` control may not display it as a line break on screen.
  - The conversion logic run by the "Run" button (`ConvertText` / `SplitLines`) handles `CRLF`, `LF`, and `CR` alike, so the converted result in the output box is not affected.
  - Workaround: after pasting, place the cursor at the end of the affected line and press `Enter` again; the line break will then also display correctly on screen.

### License

Released under the [MIT License](LICENSE).
