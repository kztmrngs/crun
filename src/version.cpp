#include "version.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

const wchar_t* CRUN_VERSION_STR = L"0.9.0";

void print_version(const wchar_t* compiler_name_arg) {
    wprintf(L"crun version %ls\n", CRUN_VERSION_STR);

    // --compiler オプションで指定されたコンパイラ名、またはデフォルトの "gcc" を使用
    const wchar_t* compiler_to_check = (compiler_name_arg && wcslen(compiler_name_arg) > 0) ? compiler_name_arg : L"gcc";

    wchar_t compiler_exe_name[20];
    swprintf_s(compiler_exe_name, 20, L"%s.exe", compiler_to_check);

    wchar_t compiler_path[MAX_PATH];
    if (find_executable_in_path(compiler_exe_name, compiler_path, MAX_PATH)) {
        wchar_t command[MAX_PATH + 20];
        swprintf_s(command, MAX_PATH + 20, L"\"%s\" --version", compiler_path);
        
        wchar_t* output = NULL;
            if (run_process_and_capture_output(command, &output) && output) {
                wchar_t* context = NULL;
                wchar_t* first_line = wcstok_s(output, L"\r\n", &context);
                if (first_line) {
                    wprintf(L"%ls\n", first_line);
                }
                free(output);
            } else {
                fwprintf_err(L"%s からバージョン情報を取得できませんでした。\n", compiler_exe_name);
            }
    } else {
        fwprintf_err(L"コンパイラ '%s' がPATHに見つかりません。\n", compiler_exe_name);
    }
}
