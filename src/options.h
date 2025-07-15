#include <windows.h>
#pragma once

#include <windows.h>

// --- プログラム設定を保持する構造体 ---
struct ProgramOptions {
    wchar_t** source_files;    // ソースファイルパスの配列
    int num_source_files;      // ソースファイルの数
    wchar_t* compiler_flags;   // コンパイラフラグ
    wchar_t* user_libraries;   // ユーザー指定ライブラリ
    wchar_t** program_args;    // プログラム引数
    int num_program_args;      // プログラム引数の数
    const wchar_t* compiler_name; // コンパイラ名 ("gcc" or "clang")
    BOOL keep_temp;            // 一時ディレクトリを保持するか
    BOOL verbose;              // 詳細出力を有効にするか
    BOOL measure_time;         // 実行時間を計測するか
    BOOL warnings_all;         // 全ての警告を有効にするか
    BOOL debug_build;          // デバッグビルドを有効にするか
};

// --- 関数宣言 ---
void print_help();
BOOL parse_arguments(int argc, wchar_t** argv, ProgramOptions* opts);
void free_options(ProgramOptions* opts);
