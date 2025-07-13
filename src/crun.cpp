#define UNICODE
#define _UNICODE

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "utils.h"
#include "version.h"
#include "options.h"
#include "compiler.h"

// --- Global State for Cleanup ---
wchar_t g_temp_dir_to_clean[MAX_PATH] = {0};
BOOL g_keep_temp = FALSE;

// --- Console Control Handler ---
BOOL WINAPI ConsoleCtrlHandler(DWORD ctrl_type) {
    if (ctrl_type == CTRL_C_EVENT || ctrl_type == CTRL_BREAK_EVENT || ctrl_type == CTRL_CLOSE_EVENT) {
        if (!g_keep_temp && g_temp_dir_to_clean[0] != L'\0') {
            remove_directory_recursively(g_temp_dir_to_clean);
        }
    }
    return FALSE;
}

// --- Main Entry Point ---
int main() {
    SetConsoleCtrlHandler(ConsoleCtrlHandler, TRUE);

    int argc;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == NULL) { return 1; }

    if (argc == 2 && wcscmp(argv[1], L"--clean") == 0) {
        wchar_t current_dir[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, current_dir);
        clean_temp_directories(current_dir);
        LocalFree(argv);
        return 0;
    }

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

    ProgramOptions opts;
    if (!parse_arguments(argc, argv, &opts)) {
        free_options(&opts);
        LocalFree(argv);
        return 1;
    }

    wchar_t main_source_full_path[MAX_PATH];
    if (!GetFullPathNameW(opts.source_files[0], MAX_PATH, main_source_full_path, NULL)) {
        fwprintf_err(L"Error: Could not get full path for source file: %s\n", opts.source_files[0]);
        free_options(&opts);
        LocalFree(argv);
        return 1;
    }

    BOOL has_cpp = FALSE;
    for (int i = 0; i < opts.num_source_files; ++i) {
        const wchar_t* ext = get_extension(opts.source_files[i]);
        if (ext && wcscmp(ext, L".cpp") == 0) {
            has_cpp = TRUE;
            break;
        }
    }

    wchar_t source_dir[MAX_PATH];
    get_parent_path(main_source_full_path, source_dir, MAX_PATH);
    wchar_t temp_dir[MAX_PATH];
    swprintf_s(temp_dir, MAX_PATH, L"%s\\crun_tmp_%lu_%lu", source_dir, GetTickCount(), GetCurrentProcessId());
    if (!CreateDirectoryW(temp_dir, NULL)) {
        fwprintf_err(L"Error: Failed to create temporary directory.\n");
        free_options(&opts);
        LocalFree(argv);
        return 1;
    }

    wcsncpy_s(g_temp_dir_to_clean, MAX_PATH, temp_dir, _TRUNCATE);
    g_keep_temp = opts.keep_temp;

    wchar_t source_stem[MAX_PATH];
    get_stem(main_source_full_path, source_stem, MAX_PATH);
    wchar_t executable_path[MAX_PATH];
    swprintf_s(executable_path, MAX_PATH, L"%s\\%s.exe", temp_dir, source_stem);

    wchar_t compiler_path[MAX_PATH];
    if (!find_compiler(opts.compiler_name, has_cpp, compiler_path, MAX_PATH)) {
        if (!opts.keep_temp) remove_directory_recursively(temp_dir);
        free_options(&opts);
        LocalFree(argv);
        return 1;
    }

    wchar_t compile_command[32767];
    if (!build_compile_command(&opts, executable_path, compiler_path, has_cpp, compile_command, 32767)) {
        if (!opts.keep_temp) remove_directory_recursively(temp_dir);
        free_options(&opts);
        LocalFree(argv);
        return 1;
    }

    if (opts.verbose) wprintf(L"--- Compiling ---\nCommand: %s\n", compile_command);
    if (!run_process(compile_command, opts.verbose)) {
        fwprintf_err(L"Compilation failed.\n");
        if (!opts.keep_temp) remove_directory_recursively(temp_dir);
        free_options(&opts);
        LocalFree(argv);
        return 1;
    }
    if (opts.verbose) wprintf(L"Compilation successful.\n");

    wchar_t run_command[32767];
    swprintf_s(run_command, 32767, L"\"%s\"", executable_path);
    for (int i = 0; i < opts.num_program_args; ++i) {
        wcscat_s(run_command, 32767, L" \"");
        wcscat_s(run_command, 32767, opts.program_args[i]);
        wcscat_s(run_command, 32767, L"\"");
    }

    if (opts.verbose) { wprintf(L"--- Running ---\n"); fflush(stdout); }

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

    if (!opts.keep_temp) remove_directory_recursively(temp_dir);
    free_options(&opts);
    LocalFree(argv);
    return exit_code;
}