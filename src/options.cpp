#define _CRT_SECURE_NO_WARNINGS
#include "options.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- ヘルプとバージョン ---
void print_help() {
    wprintf(
        L"crun - C/C++を手軽に実行するツール\n\n"
        L"使用法:\n"
        L"    crun <source_file> [program_arguments...] [options...]\n"
        L"    crun --clean\n\n"
        L"オプション:\n"
        L"    --help              このヘルプメッセージを表示します。\n"
        L"    --version           バージョン情報を表示します。\n"
        L"    --compiler <name>   コンパイラを指定します ('gcc' または 'clang')。デフォルト: 'gcc'。\n"
        L"    --cflags \"<flags>\"  コンパイラに追加のフラグを渡します。\n"
        L"    --libs \"<libs>\"      追加のライブラリとリンクします (例: \"-luser32 -lgdi32\")。\n"
        L"    --keep-temp         実行後に一時ディレクトリを保持します。\n"
        L"    --verbose, -v       詳細な出力を有効にします。\n"
        L"    --time              実行時間を計測して表示します。\n"
        L"    --debug, -g         デバッグビルドを有効にします (-g)。\n"
        L"    --wall              コンパイラの全ての警告を有効にします (-Wall)。\n"
        L"    --clean             現在いるディレクトリから一時ディレクトリ (crun_tmp_*) を削除します。\n"
    );
}

// --- 引数解析 ---
BOOL parse_arguments(int argc, wchar_t** argv, ProgramOptions* opts) {
    memset(opts, 0, sizeof(ProgramOptions));
    opts->compiler_name = L"gcc"; // Default compiler
    opts->source_files = (wchar_t**)malloc(sizeof(wchar_t*) * argc);
    opts->program_args = (wchar_t**)malloc(sizeof(wchar_t*) * argc);
    if (!opts->source_files || !opts->program_args) {
        fwprintf_err(L"エラー: 引数のためのメモリ確保に失敗しました。\n");
        if (opts->source_files) free(opts->source_files);
        if (opts->program_args) free(opts->program_args);
        return FALSE;
    }

    BOOL cflags_next = FALSE;
    BOOL libs_next = FALSE;
    BOOL compiler_next = FALSE;
    BOOL sources_ended = FALSE; // Flag to indicate that the list of source files has ended

    for (int i = 1; i < argc; ++i) {
        wchar_t* arg = argv[i];

        if (cflags_next) { opts->compiler_flags = arg; cflags_next = FALSE; continue; }
        if (libs_next) { opts->user_libraries = arg; libs_next = FALSE; continue; }
        if (compiler_next) {
            if (wcscmp(arg, L"gcc") == 0 || wcscmp(arg, L"clang") == 0) {
                opts->compiler_name = arg;
            } else {
                fwprintf_err(L"エラー: 無効なコンパイラです。'gcc' または 'clang' を使用してください。\n");
                return FALSE;
            }
            compiler_next = FALSE;
            continue;
        }

        if (wcscmp(arg, L"--help") == 0) { print_help(); return FALSE; } // ヘルプのための特別ケース
        if (wcscmp(arg, L"--version") == 0) { /* mainで処理 */ continue; }
        if (wcscmp(arg, L"--keep-temp") == 0) { opts->keep_temp = TRUE; continue; }
        if (wcscmp(arg, L"--verbose") == 0 || wcscmp(arg, L"-v") == 0) { opts->verbose = TRUE; continue; }
        if (wcscmp(arg, L"--time") == 0) { opts->measure_time = TRUE; continue; }
        if (wcscmp(arg, L"--wall") == 0) { opts->warnings_all = TRUE; continue; }
        if (wcscmp(arg, L"--debug") == 0 || wcscmp(arg, L"-g") == 0) { opts->debug_build = TRUE; continue; }
        if (wcscmp(arg, L"--clean") == 0) { /* mainで処理 */ continue; }
        if (wcscmp(arg, L"--cflags") == 0) { cflags_next = TRUE; continue; }
        if (wcscmp(arg, L"--libs") == 0) { libs_next = TRUE; continue; }
        if (wcscmp(arg, L"--compiler") == 0) { compiler_next = TRUE; continue; }

        if (wcsncmp(arg, L"--", 2) == 0) {
            fwprintf_err(L"エラー: 不明なオプション '%s' です。\n", arg);
            return FALSE;
        }

        const wchar_t* ext = get_extension(arg);
        if (!sources_ended && ext && (wcscmp(ext, L".c") == 0 || wcscmp(ext, L".cpp") == 0)) {
            opts->source_files[opts->num_source_files++] = arg;
        } else {
            sources_ended = TRUE;
            opts->program_args[opts->num_program_args++] = arg;
        }
    }

    if (cflags_next || libs_next || compiler_next) { 
        fwprintf_err(L"エラー: オプションには引数が必要です。\n"); 
        return FALSE; 
    }
    if (opts->num_source_files == 0) { 
        fwprintf_err(L"エラー: ソースファイルが指定されていません。\n"); 
        print_help();
        return FALSE; 
    }

    return TRUE;
}

void free_options(ProgramOptions* opts) {
    if (opts) {
        if (opts->source_files) free(opts->source_files);
        if (opts->program_args) free(opts->program_args);
    }
}