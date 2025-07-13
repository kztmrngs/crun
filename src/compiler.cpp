#include "compiler.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>

// --- Compiler Setup ---
// --- コンパイラの設定 ---
BOOL find_compiler(const wchar_t* compiler_name, BOOL has_cpp, wchar_t* compiler_path, size_t path_size) {
    wchar_t compiler_exe_name[20];
    if (has_cpp) {
        wcscpy_s(compiler_exe_name, 20, (wcscmp(compiler_name, L"gcc") == 0) ? L"g++.exe" : L"clang++.exe");
    } else {
        wcscpy_s(compiler_exe_name, 20, (wcscmp(compiler_name, L"gcc") == 0) ? L"gcc.exe" : L"clang.exe");
    }

    if (!find_executable_in_path(compiler_exe_name, compiler_path, path_size)) {
        fwprintf_err(L"Error: Compiler '%s' not found in PATH.\n" L"Please make sure MinGW (for gcc/g++) or Clang is installed and its 'bin' directory is in the system's PATH environment variable.\n", compiler_exe_name);
        return FALSE;
    }
    return TRUE;
}

// --- Compilation ---
// --- コンパイル ---
BOOL build_compile_command(const ProgramOptions* opts, const wchar_t* executable_path, const wchar_t* compiler_path, BOOL has_cpp, wchar_t* command, size_t command_size) {
    wchar_t all_source_files_str[32767];
    all_source_files_str[0] = L'\0';

    for (int i = 0; i < opts->num_source_files; ++i) {
        wchar_t full_path[MAX_PATH];
        if (!GetFullPathNameW(opts->source_files[i], MAX_PATH, full_path, NULL)) {
            fwprintf_err(L"Error: Could not get full path for source file: %s\n", opts->source_files[i]);
            return FALSE;
        }
        wcscat_s(all_source_files_str, 32767, L" \"");
        wcscat_s(all_source_files_str, 32767, full_path);
        wcscat_s(all_source_files_str, 32767, L"\"");
    }

    wchar_t auto_flags[256] = L"";
    if (opts->debug_build) {
        wcscpy_s(auto_flags, 256, L"-g");
    } else {
        wcscpy_s(auto_flags, 256, L"-O2 -s");
    }

    for (int i = 0; i < opts->num_source_files; ++i) {
        wchar_t full_path[MAX_PATH];
        GetFullPathNameW(opts->source_files[i], MAX_PATH, full_path, NULL);
        wchar_t* source_content = NULL;
        if (read_file_content_wide(full_path, &source_content)) {
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
            if (wcsstr(source_content, L"pthread.h")) { wcscat_s(auto_flags, 256, L" -lpthread"); }
            if (wcsstr(source_content, L"math.h")) { wcscat_s(auto_flags, 256, L" -lm"); }
            free(source_content);
        } else {
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

    if (opts->warnings_all) {
        wcscat_s(auto_flags, 256, L" -Wall");
    }

    swprintf_s(command, command_size, L"\"%s\" %s -o \"%s\" %s %s",
        compiler_path, all_source_files_str, executable_path, auto_flags,
        opts->compiler_flags ? opts->compiler_flags : L"");

    return TRUE;
}
