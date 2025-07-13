#include "options.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// --- Argument Parsing ---
// --- 引数解析 ---
BOOL parse_arguments(int argc, wchar_t** argv, ProgramOptions* opts) {
    memset(opts, 0, sizeof(ProgramOptions));
    opts->compiler_name = L"gcc"; // デフォルトコンパイラ
    opts->source_files = (wchar_t**)malloc(sizeof(wchar_t*) * argc);
    opts->program_args = (wchar_t**)malloc(sizeof(wchar_t*) * argc);
    if (!opts->source_files || !opts->program_args) {
        fwprintf_err(L"Error: Failed to allocate memory for arguments.\n");
        if (opts->source_files) free(opts->source_files);
        if (opts->program_args) free(opts->program_args);
        return FALSE;
    }

    BOOL cflags_next = FALSE;
    BOOL compiler_next = FALSE;
    BOOL sources_ended = FALSE; // ソースファイルのリストが終了したかを示すフラグ

    for (int i = 1; i < argc; ++i) {
        wchar_t* arg = argv[i];

        if (cflags_next) { opts->compiler_flags = arg; cflags_next = FALSE; continue; }
        if (compiler_next) {
            if (wcscmp(arg, L"gcc") == 0 || wcscmp(arg, L"clang") == 0) {
                opts->compiler_name = arg;
            } else {
                fwprintf_err(L"Error: Invalid compiler. Use 'gcc' or 'clang'.\n");
                return FALSE;
            }
            compiler_next = FALSE;
            continue;
        }

        if (wcscmp(arg, L"--help") == 0) { print_help(); return FALSE; } // Special case for help
        if (wcscmp(arg, L"--version") == 0) { /* Handled in main */ continue; }
        if (wcscmp(arg, L"--keep-temp") == 0) { opts->keep_temp = TRUE; continue; }
        if (wcscmp(arg, L"--verbose") == 0 || wcscmp(arg, L"-v") == 0) { opts->verbose = TRUE; continue; }
        if (wcscmp(arg, L"--time") == 0) { opts->measure_time = TRUE; continue; }
        if (wcscmp(arg, L"--wall") == 0) { opts->warnings_all = TRUE; continue; }
        if (wcscmp(arg, L"--debug") == 0 || wcscmp(arg, L"-g") == 0) { opts->debug_build = TRUE; continue; }
        if (wcscmp(arg, L"--clean") == 0) { /* Handled in main */ continue; }
        if (wcscmp(arg, L"--cflags") == 0) { cflags_next = TRUE; continue; }
        if (wcscmp(arg, L"--compiler") == 0) { compiler_next = TRUE; continue; }

        if (wcsncmp(arg, L"--", 2) == 0) {
            fwprintf_err(L"Error: Unknown option '%s'.\n", arg);
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

    if (cflags_next || compiler_next) { 
        fwprintf_err(L"Error: Option requires an argument.\n"); 
        return FALSE; 
    }
    if (opts->num_source_files == 0) { 
        fwprintf_err(L"Error: No source files specified.\n"); 
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
