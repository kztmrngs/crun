// crun.cpp - A simple C/C++ runner
// Rewritten to use C standard library and Win32 API for minimal executable size.
// crun.cpp - 単純なC/C++実行ツール
// 実行ファイルのサイズを最小化するため、C標準ライブラリとWin32 APIを使って書き直されています。

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <string.h>

#include "utils.h"
#include "version.h"

// --- Global State for Cleanup ---
// --- クリーンアップ用のグローバル変数 ---
wchar_t g_temp_dir_to_clean[MAX_PATH] = {0};
BOOL g_keep_temp = FALSE;

// --- Console Control Handler ---
// --- コンソール制御ハンドラ ---
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrl_type) {
    // Ctrl+C, Ctrl+Break, Close eventを捕捉
    if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_BREAK_EVENT || ctrl_type == CTRL_CLOSE_EVENT) {
        if (!g_keep_temp && g_temp_dir_to_clean[0] != L'\0') {
            remove_directory_recursively(g_temp_dir_to_clean);
        }
    }
    return FALSE; // 次のハンドラに制御を渡す
}

// --- Options Structure ---
// --- プログラム設定を保持する構造体 ---
struct ProgramOptions {
    wchar_t** source_files;    // ソースファイルパスの配列
    int num_source_files;      // ソースファイルの数
    wchar_t* compiler_flags;   // コンパイラフラグ
    wchar_t** program_args;    // プログラム引数
    int num_program_args;      // プログラム引数の数
    const wchar_t* compiler_name; // コンパイラ名 ("gcc" or "clang")
    BOOL keep_temp;            // 一時ディレクトリを保持するか
    BOOL verbose;              // 詳細出力を有効にするか
    BOOL measure_time;         // 実行時間を計測するか
    BOOL warnings_all;         // 全ての警告を有効にするか
    BOOL debug_build;          // デバッグビルドを有効にするか
};

// --- Help and Version ---
// --- ヘルプとバージョン情報を表示する関数 ---
void print_help() {
    wprintf(
        L"crun - A simple C/C++ runner.\n\n"
        L"USAGE:\n"
        L"    crun <source_file> [program_arguments...] [options...]\n"
        L"    crun --clean\n\n"
        L"OPTIONS:\n"
        L"    --help              Show this help message.\n"
        L"    --version           Show version information.\n"
        L"    --compiler <name>   Specify the compiler ('gcc' or 'clang'). Default: 'gcc'.\n"
        L"    --cflags \"<flags>\"  Pass additional flags to the compiler.\n"
        L"    --keep-temp         Keep the temporary directory after execution.\n"
        L"    --verbose, -v       Enable verbose output.\n"
        L"    --time              Measure and show the execution time.\n"
        L"    --wall              Enable all compiler warnings (-Wall).\n"
        L"    --debug, -g         Enable debug build (-g).\n"
        L"    --clean             Remove temporary directories (crun_tmp_*) from the current directory.\n"
    );
}

// --- Main Entry Point ---
// --- メインエントリーポイント ---
int main() {
    // コンソール制御ハンドラを設定
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    int argc;
    // コマンドライン引数をワイド文字列として取得
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == NULL) { return 1; } // Should not happen

    // --clean オプションを特別に処理
    if (argc == 2 && wcscmp(argv[1], L"--clean") == 0) {
        wchar_t current_dir[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, current_dir);
        clean_temp_directories(current_dir);
        LocalFree(argv);
        return 0;
    }

    // --- Pre-parse for --version and --compiler ---
    // --- --version と --compiler のための事前解析 ---
    const wchar_t* specified_compiler = L"";
    for (int i = 1; i < argc - 1; ++i) {
        if (wcscmp(argv[i], L"--compiler") == 0) {
            specified_compiler = argv[i + 1];
            break;
        }
    }

    if (argc >= 2 && wcscmp(argv[1], L"--version") == 0) {
        print_version(specified_compiler);
        LocalFree(argv);
        return 0;
    }


    if (argc < 2) {
        print_help();
        LocalFree(argv);
        return 1;
    }

    // --- Argument Parsing ---
    // --- 引数解析 ---
    ProgramOptions opts = {0};
    opts.compiler_name = L"gcc"; // デフォルトコンパイラ
    opts.source_files = (wchar_t**)malloc(sizeof(wchar_t*) * argc);
    opts.program_args = (wchar_t**)malloc(sizeof(wchar_t*) * argc);
    if (!opts.source_files || !opts.program_args) {
        fwprintf_err(L"Error: Failed to allocate memory for arguments.\n");
        if (opts.source_files) free(opts.source_files);
        if (opts.program_args) free(opts.program_args);
        LocalFree(argv);
        return 1;
    }

    BOOL cflags_next = FALSE;
    BOOL compiler_next = FALSE;
    BOOL sources_ended = FALSE; // ソースファイルのリストが終了したかを示すフラグ

    for (int i = 1; i < argc; ++i) {
        wchar_t* arg = argv[i];

        if (cflags_next) { opts.compiler_flags = arg; cflags_next = FALSE; continue; }
        if (compiler_next) {
            if (wcscmp(arg, L"gcc") == 0 || wcscmp(arg, L"clang") == 0) {
                opts.compiler_name = arg;
            } else {
                fwprintf_err(L"Error: Invalid compiler. Use 'gcc' or 'clang'.\n");
                free(opts.source_files); free(opts.program_args);
                LocalFree(argv);
                return 1;
            }
            compiler_next = FALSE;
            continue;
        }

        if (wcscmp(arg, L"--help") == 0) { print_help(); free(opts.source_files); free(opts.program_args); LocalFree(argv); return 0; }
        if (wcscmp(arg, L"--version") == 0) { /* Handled above */ continue; }
        if (wcscmp(arg, L"--keep-temp") == 0) { opts.keep_temp = TRUE; continue; }
        if (wcscmp(arg, L"--verbose") == 0 || wcscmp(arg, L"-v") == 0) { opts.verbose = TRUE; continue; }

        if (wcscmp(arg, L"--time") == 0) { opts.measure_time = TRUE; continue; }
        if (wcscmp(arg, L"--wall") == 0) { opts.warnings_all = TRUE; continue; }
        if (wcscmp(arg, L"--debug") == 0 || wcscmp(arg, L"-g") == 0) { opts.debug_build = TRUE; continue; }
        if (wcscmp(arg, L"--clean") == 0) { continue; } // Special handling at the start
        if (wcscmp(arg, L"--cflags") == 0) { cflags_next = TRUE; continue; }
        if (wcscmp(arg, L"--compiler") == 0) { compiler_next = TRUE; continue; }

        // オプションかどうかを判定
        if (wcsncmp(arg, L"--", 2) == 0) {
            fwprintf_err(L"Error: Unknown option '%s'.\n", arg);
            free(opts.source_files); free(opts.program_args);
            LocalFree(argv);
            return 1;
        }

        // .c または .cpp で終わる引数をソースファイルとして解釈
        const wchar_t* ext = get_extension(arg);
        if (!sources_ended && ext && (wcscmp(ext, L".c") == 0 || wcscmp(ext, L".cpp") == 0)) {
            opts.source_files[opts.num_source_files++] = arg;
        } else {
            // 最初の非ソースファイル以降はすべてプログラム引数とみなす
            sources_ended = TRUE;
            opts.program_args[opts.num_program_args++] = arg;
        }
    }

    if (cflags_next || compiler_next) { fwprintf_err(L"Error: Option requires an argument.\n"); free(opts.source_files); free(opts.program_args); LocalFree(argv); return 1; }
    if (opts.num_source_files == 0) { fwprintf_err(L"Error: No source files specified.\n"); print_help(); free(opts.source_files); free(opts.program_args); LocalFree(argv); return 1;
    }

    // --- Path and File Setup ---
    // --- パスとファイルの設定 ---
    wchar_t main_source_full_path[MAX_PATH]; // 最初のソースファイルのフルパス（一時ディレクトリの場所を決めるため）
    if (!GetFullPathNameW(opts.source_files[0], MAX_PATH, main_source_full_path, NULL)) {
        fwprintf_err(L"Error: Could not get full path for source file: %s\n", opts.source_files[0]);
        free(opts.source_files); free(opts.program_args);
        LocalFree(argv);
        return 1;
    }

    BOOL has_cpp = FALSE;
    wchar_t all_source_files_str[32767];
    all_source_files_str[0] = L'\0';

    for (int i = 0; i < opts.num_source_files; ++i) {
        wchar_t full_path[MAX_PATH];
        if (!GetFullPathNameW(opts.source_files[i], MAX_PATH, full_path, NULL)) {
            fwprintf_err(L"Error: Could not get full path for source file: %s\n", opts.source_files[i]);
            free(opts.source_files); free(opts.program_args); LocalFree(argv); return 1;
        }
        if (!file_exists(full_path)) {
            fwprintf_err(L"Error: Source file not found: %s\n", full_path);
            free(opts.source_files); free(opts.program_args); LocalFree(argv); return 1;
        }
        const wchar_t* ext = get_extension(full_path);
        if (!ext || (wcscmp(ext, L".c") != 0 && wcscmp(ext, L".cpp") != 0)) {
            fwprintf_err(L"Error: Unsupported file type: %s. Only .c and .cpp are supported.\n", opts.source_files[i]);
            free(opts.source_files); free(opts.program_args); LocalFree(argv); return 1;
        }
        if (wcscmp(ext, L".cpp") == 0) has_cpp = TRUE;

        // Add to the string for the compile command
        wcscat_s(all_source_files_str, 32767, L" \"");
        wcscat_s(all_source_files_str, 32767, full_path);
        wcscat_s(all_source_files_str, 32767, L"\"");
    }

    // 一時ディレクトリを作成
    wchar_t source_dir[MAX_PATH];
    get_parent_path(main_source_full_path, source_dir, MAX_PATH);
    wchar_t temp_dir[MAX_PATH];
    swprintf_s(temp_dir, MAX_PATH, L"%s\\crun_tmp_%lu_%lu", source_dir, GetTickCount(), GetCurrentProcessId());
    if (!CreateDirectoryW(temp_dir, NULL)) {
        fwprintf_err(L"Error: Failed to create temporary directory.\n");
        free(opts.program_args);
        LocalFree(argv);
        return 1;
    }

    // グローバル変数に情報を保存
    wcsncpy_s(g_temp_dir_to_clean, MAX_PATH, temp_dir, _TRUNCATE);
    g_keep_temp = opts.keep_temp;

    // 実行ファイルパスを生成
    wchar_t source_stem[MAX_PATH];
    get_stem(main_source_full_path, source_stem, MAX_PATH);
    wchar_t executable_path[MAX_PATH];
    swprintf_s(executable_path, MAX_PATH, L"%s\\%s.exe", temp_dir, source_stem);

    // --- Compiler Setup ---
    // --- コンパイラの設定 ---
    wchar_t compiler_exe_name[20];
    // 拡張子に応じてコンパイラ実行ファイル名を決定 (一つでも.cppがあればC++コンパイラ)
    if (has_cpp) {
        wcscpy_s(compiler_exe_name, 20, (wcscmp(opts.compiler_name, L"gcc") == 0) ? L"g++.exe" : L"clang++.exe");
    } else {
        wcscpy_s(compiler_exe_name, 20, (wcscmp(opts.compiler_name, L"gcc") == 0) ? L"gcc.exe" : L"clang.exe");
    }

    // PATH環境変数からコンパイラのフルパスを検索
    wchar_t compiler_path[MAX_PATH];
    if (!find_executable_in_path(compiler_exe_name, compiler_path, MAX_PATH)) {
        fwprintf_err(L"Error: Compiler '%s' not found in PATH.\n" L"Please make sure MinGW (for gcc/g++) or Clang is installed and its 'bin' directory is in the system's PATH environment variable.\n", compiler_exe_name);
        if (!opts.keep_temp) remove_directory_recursively(temp_dir);
        free(opts.program_args);
        LocalFree(argv);
        return 1;
    }

    // --- Compilation ---
    // --- コンパイル ---
    wchar_t compile_command[32767] = {0};
    wchar_t auto_flags[256] = L""; // 自動フラグ

    // ビルドの種類に応じてフラグを設定
    if (opts.debug_build) {
        wcscpy_s(auto_flags, 256, L"-g"); // デバッグ情報
    } else {
        wcscpy_s(auto_flags, 256, L"-O2 -s"); // リリースビルド用の最適化
    }

    // ソースコードの内容を読み込む
    for (int i = 0; i < opts.num_source_files; ++i) {
        wchar_t full_path[MAX_PATH];
        GetFullPathNameW(opts.source_files[i], MAX_PATH, full_path, NULL);
        wchar_t* source_content = NULL;
        if (read_file_content_wide(full_path, &source_content)) {
            // Windows APIヘッダのインクルードをチェック (ファイル名のみで検索)
            if (wcsstr(source_content, L"windows.h")) { wcscat_s(auto_flags, 256, L" -lkernel32 -luser32 -lshell32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32"); }
            if (wcsstr(source_content, L"winsock2.h") || wcsstr(source_content, L"winsock.h")) { wcscat_s(auto_flags, 256, L" -lws2_32"); }
            if (wcsstr(source_content, L"shlobj.h") || wcsstr(source_content, L"ole32.h")) { wcscat_s(auto_flags, 256, L" -lole32"); }
            if (wcsstr(source_content, L"dwmapi.h")) { wcscat_s(auto_flags, 256, L" -ldwmapi"); }
            if (wcsstr(source_content, L"wininet.h")) { wcscat_s(auto_flags, 256, L" -lwininet"); }
            if (wcsstr(source_content, L"comctl32.h")) { wcscat_s(auto_flags, 256, L" -lcomctl32"); }
            if (wcsstr(source_content, L"version.h")) { wcscat_s(auto_flags, 256, L" -lversion"); }
            if (wcsstr(source_content, L"rpc.h")) { wcscat_s(auto_flags, 256, L" -lrpcrt4"); }
            if (wcsstr(source_content, L"bcrypt.h")) { wcscat_s(auto_flags, 256, L" -lbcrypt"); }
            if (wcsstr(source_content, L"ncrypt.h")) { wcscat_s(auto_flags, 256, L" -lncrypt"); }
            if (wcsstr(source_content, L"d3d11.h")) { wcscat_s(auto_flags, 256, L" -ld3d11"); }
            if (wcsstr(source_content, L"d2d1.h")) { wcscat_s(auto_flags, 256, L" -ld2d1"); }
            if (wcsstr(source_content, L"dwrite.h")) { wcscat_s(auto_flags, 256, L" -ldwrite"); }

            // 標準ライブラリヘッダのインクルードをチェック
            if (wcsstr(source_content, L"pthread.h")) { wcscat_s(auto_flags, 256, L" -lpthread"); }
            if (wcsstr(source_content, L"math.h")) { wcscat_s(auto_flags, 256, L" -lm"); }

            free(source_content); // メモリを解放
        } else {
            // ファイルが読み込めない場合、従来のヘッダ依存性チェックにフォールバック
            wchar_t dep_command[MAX_PATH * 2];
            swprintf_s(dep_command, MAX_PATH * 2, L"\"%s\" -MM %s", compiler_path, all_source_files_str);
            wchar_t* dep_output = NULL;
            if (run_process_and_capture_output(dep_command, &dep_output) && dep_output) {
                if (wcsstr(dep_output, L"pthread.h")) { wcscat_s(auto_flags, 256, L" -lpthread"); }
                if (wcsstr(dep_output, L"math.h")) { wcscat_s(auto_flags, 256, L" -lm"); }
                free(dep_output);
            }
        }
    }

    // 警告フラグを追加
    if (opts.warnings_all) {
        wcscat_s(auto_flags, 256, L" -Wall");
    }

    // 最終的なコンパイルコマンドを構築
    swprintf_s(compile_command, 32767, L"\"%s\" %s -o \"%s\" %s %s",
        compiler_path, all_source_files_str, executable_path, auto_flags,
        opts.compiler_flags ? opts.compiler_flags : L"");

    if (opts.verbose) wprintf(L"--- Compiling ---\nCommand: %s\n", compile_command);
    if (!run_process(compile_command, opts.verbose)) {
        fwprintf_err(L"Compilation failed.\n");
        if (!opts.keep_temp) remove_directory_recursively(temp_dir);
        free(opts.program_args);
        LocalFree(argv);
        return 1;
    }
    if (opts.verbose) wprintf(L"Compilation successful.\n");

    // --- Execution ---
    // --- 実行 ---
    wchar_t run_command[32767];
    swprintf_s(run_command, 32767, L"\"%s\"", executable_path);
    // プログラム引数をコマンドラインに追加
    for (int i = 0; i < opts.num_program_args; ++i) {
        wcscat_s(run_command, 32767, L" \"");
        wcscat_s(run_command, 32767, opts.program_args[i]);
        wcscat_s(run_command, 32767, L"\"");
    }

    if (opts.verbose) { wprintf(L"--- Running ---\n"); fflush(stdout); }

    // 実行時間を計測
    LARGE_INTEGER start_time, end_time, frequency;
    if (opts.measure_time) { QueryPerformanceFrequency(&frequency); QueryPerformanceCounter(&start_time); }

    DWORD exit_code = 0;
    run_program_and_get_exit_code(run_command, &exit_code);

    if (opts.measure_time) {
        QueryPerformanceCounter(&end_time);
        double elapsed_ms = (double)(end_time.QuadPart - start_time.QuadPart) * 1000.0 / frequency.QuadPart;
        wprintf(L"\nExecution time: %.3f ms\n", elapsed_ms);
    }
    if (opts.verbose) wprintf(L"\n--- Finished ---\nProgram exited with code %lu.\n", exit_code);

    // --- Cleanup ---
    // --- クリーンアップ ---
    if (!opts.keep_temp) remove_directory_recursively(temp_dir);
    free(opts.program_args);
    LocalFree(argv);
    return exit_code;
}
