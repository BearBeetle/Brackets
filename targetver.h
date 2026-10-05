#pragma once

// Windows プラットフォームで利用可能な最新機能を有効にする。
// 古い Windows バージョンをサポートする必要がある場合は、
// WIN32_WINNT を対象とする最小プラットフォームに合わせて変更する。

#ifndef WINVER
#define WINVER 0x0A00       // Windows 10 以降
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 // Windows 10 以降
#endif