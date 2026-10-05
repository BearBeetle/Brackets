#include "targetver.h"

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <string>
#include <vector>
#include <memory>

#include "resource.h"

// ==============================================================
// グローバル定数
// ==============================================================

// INI ファイル名
static const wchar_t* const INI_FILE_NAME = L"Backets.ini";

// INI セクション名／キー名
static const wchar_t* const INI_SECTION = L"Settings";
static const wchar_t* const INI_KEY_INDENT = L"IndentString";

// インデント記号の初期値（半角スペース4個 + '>'）
static const wchar_t* const DEFAULT_INDENT = L"    >";

// 初期ウィンドウサイズ（ピクセル）
static const int INITIAL_WINDOW_WIDTH = 1250;
static const int INITIAL_WINDOW_HEIGHT = 550;

// ==============================================================
// 関数プロトタイプ
// ==============================================================

std::wstring GetIniPath();
std::wstring LoadIndentString();
bool SaveIndentString(const std::wstring& indent);
std::vector<std::wstring> SplitLines(const std::wstring& text);
std::wstring ConvertText(const std::wstring& source, const std::wstring& indent);

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
static void OnInitDialog(HWND hDlg);
static void OnExecute(HWND hDlg);
static void OnResize(HWND hDlg, int width, int height);
static std::wstring GetWindowTextAsWString(HWND hwnd);
static void SetWindowTextFromWString(HWND hwnd, const std::wstring& text);

// ==============================================================
// エントリポイント
// ==============================================================

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE hPrevInstance,
                       _In_ LPWSTR lpCmdLine,
                       _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    // ダイアログボックスをモーダルとして起動する
    INT_PTR result = DialogBoxParamW(hInstance,
                                      MAKEINTRESOURCEW(IDD_BACKETS),
                                      nullptr,
                                      DialogProc,
                                      0);

    if (result == -1)
    {
        MessageBoxW(nullptr, L"ダイアログの作成に失敗しました。", L"Backets", MB_OK | MB_ICONERROR);
        return 1;
    }

    return 0;
}

// ==============================================================
// INI ファイルパス取得
//   実行ファイルと同じフォルダの Backets.ini を指す絶対パスを返す。
//   カレントディレクトリには依存しない。
// ==============================================================
std::wstring GetIniPath()
{
    wchar_t modulePath[MAX_PATH] = { 0 };

    DWORD length = GetModuleFileNameW(nullptr, modulePath, MAX_PATH);
    if (length == 0 || length == MAX_PATH)
    {
        // 取得失敗時はカレントディレクトリ相対のファイル名を返す（フォールバック）
        return INI_FILE_NAME;
    }

    std::wstring path(modulePath);

    // 最後の '\' を検索し、それ以降をファイル名に置き換える
    size_t pos = path.find_last_of(L'\\');
    if (pos == std::wstring::npos)
    {
        return INI_FILE_NAME;
    }

    std::wstring directory = path.substr(0, pos + 1);
    directory += INI_FILE_NAME;

    return directory;
}

// ==============================================================
// INI 読み込み
//   読み込み成功時は保存されている値を返し、
//   失敗時（ファイル不在・キー不在等）は初期値を返す。
// ==============================================================
std::wstring LoadIndentString()
{
    std::wstring iniPath = GetIniPath();

    wchar_t buffer[1024] = { 0 };

    DWORD readCount = GetPrivateProfileStringW(
        INI_SECTION,
        INI_KEY_INDENT,
        DEFAULT_INDENT,      // 既定値（見つからない場合はこれがそのままコピーされる）
        buffer,
        static_cast<DWORD>(sizeof(buffer) / sizeof(wchar_t)),
        iniPath.c_str());

    UNREFERENCED_PARAMETER(readCount);

    return std::wstring(buffer);
}

// ==============================================================
// INI 保存
//   実行ファイルと同じフォルダの Backets.ini へ IndentString を保存する。
//   成功時 true、失敗時 false を返す。
// ==============================================================
bool SaveIndentString(const std::wstring& indent)
{
    std::wstring iniPath = GetIniPath();

    BOOL result = WritePrivateProfileStringW(
        INI_SECTION,
        INI_KEY_INDENT,
        indent.c_str(),
        iniPath.c_str());

    return result != FALSE;
}

// ==============================================================
// 行分割
//   CRLF / LF / CR いずれの改行コードにも対応して行分割を行う。
//   末尾に改行が存在する場合、末尾の空行も要素として保持する。
// ==============================================================
std::vector<std::wstring> SplitLines(const std::wstring& text)
{
    std::vector<std::wstring> lines;

    if (text.empty())
    {
        return lines;
    }

    std::wstring currentLine;
    size_t i = 0;
    const size_t length = text.length();

    while (i < length)
    {
        wchar_t ch = text[i];

        if (ch == L'\r')
        {
            // CRLF または CR のいずれか
            lines.push_back(currentLine);
            currentLine.clear();

            if (i + 1 < length && text[i + 1] == L'\n')
            {
                // CRLF
                i += 2;
            }
            else
            {
                // CR のみ
                i += 1;
            }
        }
        else if (ch == L'\n')
        {
            // LF のみ
            lines.push_back(currentLine);
            currentLine.clear();
            i += 1;
        }
        else
        {
            currentLine += ch;
            i += 1;
        }
    }

    // 最後の行（末尾に改行が無い場合の残り）を追加
    lines.push_back(currentLine);

    return lines;
}

// ==============================================================
// 変換処理
//   各行の先頭にインデント記号を付加し、改行は CRLF に統一する。
//   入力が空の場合は空文字列を返す。
// ==============================================================
std::wstring ConvertText(const std::wstring& source, const std::wstring& indent)
{
    if (source.empty())
    {
        return std::wstring();
    }

    std::vector<std::wstring> lines = SplitLines(source);

    std::wstring result;

    for (size_t i = 0; i < lines.size(); ++i)
    {
        result += indent;
        result += lines[i];

        // 最後の行以外は改行を付加（末尾に余分な改行を追加しない）
        if (i + 1 < lines.size())
        {
            result += L"\r\n";
        }
    }

    return result;
}

// ==============================================================
// ウィンドウ文字列取得ヘルパー
// ==============================================================
static std::wstring GetWindowTextAsWString(HWND hwnd)
{
    int length = GetWindowTextLengthW(hwnd);
    if (length <= 0)
    {
        return std::wstring();
    }

    std::vector<wchar_t> buffer(static_cast<size_t>(length) + 1, L'\0');

    GetWindowTextW(hwnd, buffer.data(), length + 1);

    return std::wstring(buffer.data());
}

static void SetWindowTextFromWString(HWND hwnd, const std::wstring& text)
{
    SetWindowTextW(hwnd, text.c_str());
}

// ==============================================================
// ダイアログ初期化処理
// ==============================================================
static void OnInitDialog(HWND hDlg)
{
    // 初期ウィンドウサイズを設定（中央付近に配置）
    HMONITOR monitor = MonitorFromWindow(hDlg, MONITOR_DEFAULTTONEAREST);
    MONITORINFO monitorInfo = { 0 };
    monitorInfo.cbSize = sizeof(MONITORINFO);

    int posX = 0;
    int posY = 0;

    if (GetMonitorInfoW(monitor, &monitorInfo))
    {
        int workWidth = monitorInfo.rcWork.right - monitorInfo.rcWork.left;
        int workHeight = monitorInfo.rcWork.bottom - monitorInfo.rcWork.top;

        posX = monitorInfo.rcWork.left + (workWidth - INITIAL_WINDOW_WIDTH) / 2;
        posY = monitorInfo.rcWork.top + (workHeight - INITIAL_WINDOW_HEIGHT) / 2;
    }

    SetWindowPos(hDlg, nullptr, posX, posY, INITIAL_WINDOW_WIDTH, INITIAL_WINDOW_HEIGHT,
                 SWP_NOZORDER);

    // レイアウトをクライアント領域サイズに合わせて再配置
    RECT clientRect = { 0 };
    GetClientRect(hDlg, &clientRect);
    OnResize(hDlg, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top);

    // INI からインデント記号を読み込み、表示する
    std::wstring indent = LoadIndentString();
    SetWindowTextFromWString(GetDlgItem(hDlg, IDC_EDIT_INDENT), indent);

    // 初期フォーカスを入力欄へ設定
    SetFocus(GetDlgItem(hDlg, IDC_EDIT_INPUT));
}

// ==============================================================
// レイアウト再配置処理
//   入力欄を上半分、出力欄を下半分に配置する。
//   ボタン、インデント記号欄は右側に固定幅で配置する。
// ==============================================================
static void OnResize(HWND hDlg, int width, int height)
{
    const int margin = 10;
    const int rightPanelWidth = 140;
    const int labelHeight = 18;
    const int buttonHeight = 26;
    const int gap = 8;

    int leftAreaWidth = width - rightPanelWidth - margin * 3;
    if (leftAreaWidth < 100)
    {
        leftAreaWidth = 100;
    }

    int totalEditHeight = height - margin * 2 - labelHeight * 2 - gap;
    if (totalEditHeight < 100)
    {
        totalEditHeight = 100;
    }

    int inputEditHeight = totalEditHeight / 2;
    int outputEditHeight = totalEditHeight - inputEditHeight;

    // 入力ラベル・入力欄
    int y = margin;
    SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_INPUT), nullptr,
                 margin, y, leftAreaWidth, labelHeight, SWP_NOZORDER);
    y += labelHeight;
    SetWindowPos(GetDlgItem(hDlg, IDC_EDIT_INPUT), nullptr,
                 margin, y, leftAreaWidth, inputEditHeight, SWP_NOZORDER);
    y += inputEditHeight + gap;

    // 出力ラベル・出力欄
    SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_OUTPUT), nullptr,
                 margin, y, leftAreaWidth, labelHeight, SWP_NOZORDER);
    y += labelHeight;
    SetWindowPos(GetDlgItem(hDlg, IDC_EDIT_OUTPUT), nullptr,
                 margin, y, leftAreaWidth, outputEditHeight, SWP_NOZORDER);

    // 右側パネル（ボタン・インデント記号）
    int rightX = margin * 2 + leftAreaWidth;

    SetWindowPos(GetDlgItem(hDlg, IDC_BUTTON_EXECUTE), nullptr,
                 rightX, margin, rightPanelWidth, buttonHeight, SWP_NOZORDER);

    SetWindowPos(GetDlgItem(hDlg, IDC_BUTTON_EXIT), nullptr,
                 rightX, margin + buttonHeight + gap, rightPanelWidth, buttonHeight, SWP_NOZORDER);

    int indentLabelY = margin + (buttonHeight + gap) * 2 + gap * 3;
    SetWindowPos(GetDlgItem(hDlg, IDC_STATIC_INDENT), nullptr,
                 rightX, indentLabelY, rightPanelWidth, labelHeight, SWP_NOZORDER);

    SetWindowPos(GetDlgItem(hDlg, IDC_EDIT_INDENT), nullptr,
                 rightX, indentLabelY + labelHeight, rightPanelWidth, 22, SWP_NOZORDER);
}

// ==============================================================
// 実行ボタン押下処理
// ==============================================================
static void OnExecute(HWND hDlg)
{
    // 入力欄・インデント記号欄の文字列を取得
    std::wstring inputText = GetWindowTextAsWString(GetDlgItem(hDlg, IDC_EDIT_INPUT));
    std::wstring indentText = GetWindowTextAsWString(GetDlgItem(hDlg, IDC_EDIT_INDENT));

    // 変換処理
    std::wstring outputText = ConvertText(inputText, indentText);

    // 出力欄へ表示
    SetWindowTextFromWString(GetDlgItem(hDlg, IDC_EDIT_OUTPUT), outputText);

    // INI へインデント記号を保存
    if (!SaveIndentString(indentText))
    {
        MessageBoxW(hDlg,
                    L"設定ファイル（Backets.ini）への保存に失敗しました。",
                    L"Backets",
                    MB_OK | MB_ICONWARNING);
    }
}

// ==============================================================
// ダイアログプロシージャ
// ==============================================================
static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_INITDIALOG:
        OnInitDialog(hDlg);
        return TRUE;

    case WM_SIZE:
        OnResize(hDlg, LOWORD(lParam), HIWORD(lParam));
        return TRUE;

    case WM_GETMINMAXINFO:
    {
        // ウィンドウの最小サイズを制限し、コントロールの重なりを防止する
        MINMAXINFO* minMaxInfo = reinterpret_cast<MINMAXINFO*>(lParam);
        minMaxInfo->ptMinTrackSize.x = 600;
        minMaxInfo->ptMinTrackSize.y = 400;
        return TRUE;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDC_BUTTON_EXECUTE:
            OnExecute(hDlg);
            return TRUE;

        case IDC_BUTTON_EXIT:
            // INI 保存は行わず正常終了
            EndDialog(hDlg, IDOK);
            return TRUE;

        default:
            break;
        }
        break;

    case WM_CLOSE:
        // タイトルバーの閉じるボタン：INI 保存は行わず正常終了
        EndDialog(hDlg, IDOK);
        return TRUE;

    default:
        break;
    }

    return FALSE;
}