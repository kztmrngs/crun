#include "utils.h"
#include <stdarg.h>
#include <stdlib.h>
#include <shellapi.h> // For SHFileOperationW

// --- Global State for Cleanup ---
// --- クリーンアップ用のグローバル変数 ---
extern wchar_t g_temp_dir_to_clean[MAX_PATH];
extern BOOL g_keep_temp;

// 標準エラー出力に書式付きで出力
void fwprintf_err(const wchar_t* format, ...) {
    va_list args;
    va_start(args, format);
    vfwprintf(stderr, format, args);
    va_end(args);
}

// ファイルの存在を確認
BOOL file_exists(const wchar_t* path) {
    DWORD attrib = GetFileAttributesW(path);
    return (attrib != INVALID_FILE_ATTRIBUTES && !(attrib & FILE_ATTRIBUTE_DIRECTORY));
}

// プロセスを実行し、完了を待つ
BOOL run_process(wchar_t* command_line, BOOL verbose) {
    PROCESS_INFORMATION pi = {0};
    STARTUPINFOW si = {0};
    si.cb = sizeof(STARTUPINFOW);
    // verboseでない場合、コンパイラのコンソールウィンドウを非表示にする
    if (!verbose) {
        si.dwFlags |= STARTF_USESHOWWINDOW;
        si.wShowWindow = SW_HIDE;
    }
    if (!CreateProcessW(NULL, command_line, NULL, NULL, FALSE, verbose ? 0 : CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        return FALSE;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exit_code == 0;
}

// プログラムを実行し、標準入出力を引き継いで終了コードを取得
BOOL run_program_and_get_exit_code(wchar_t* command_line, DWORD* p_exit_code) {
    PROCESS_INFORMATION pi = {0};
    STARTUPINFOW si = {0};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    if (!CreateProcessW(NULL, command_line, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        return FALSE;
    }
    WaitForSingleObject(pi.hProcess, INFINITE);
    GetExitCodeProcess(pi.hProcess, p_exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return TRUE;
}

// プロセスを実行し、その標準出力をキャプチャする
BOOL run_process_and_capture_output(wchar_t* command_line, wchar_t** output) {
    HANDLE h_child_stdout_rd, h_child_stdout_wr;
    SECURITY_ATTRIBUTES sa_attr = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    if (!CreatePipe(&h_child_stdout_rd, &h_child_stdout_wr, &sa_attr, 0) || !SetHandleInformation(h_child_stdout_rd, HANDLE_FLAG_INHERIT, 0)) return FALSE;

    PROCESS_INFORMATION pi = {0};
    STARTUPINFOW si = {0};
    si.cb = sizeof(STARTUPINFOW);
    si.dwFlags |= STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;
    si.hStdOutput = h_child_stdout_wr;
    si.hStdError = h_child_stdout_wr; // 標準エラー出力もキャプチャ
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    if (!CreateProcessW(NULL, command_line, NULL, NULL, TRUE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi)) {
        CloseHandle(h_child_stdout_rd); CloseHandle(h_child_stdout_wr);
        return FALSE;
    }
    CloseHandle(h_child_stdout_wr); // 書き込みハンドルは不要なので閉じる

    // パイプから出力を読み取る
    char buffer[1024];
    DWORD bytes_read, total_size = 0;
    char* narrow_output = NULL;
    while (ReadFile(h_child_stdout_rd, buffer, sizeof(buffer), &bytes_read, NULL) && bytes_read != 0) {
        char* new_output = (char*)realloc(narrow_output, total_size + bytes_read);
        if (!new_output) { free(narrow_output); CloseHandle(h_child_stdout_rd); return FALSE; }
        narrow_output = new_output;
        memcpy(narrow_output + total_size, buffer, bytes_read);
        total_size += bytes_read;
    }
    char* final_output = (char*)realloc(narrow_output, total_size + 1);
    if (!final_output) { free(narrow_output); CloseHandle(h_child_stdout_rd); return FALSE; }
    narrow_output = final_output;
    narrow_output[total_size] = '\0';

    // UTF-8からワイド文字列に変換
    int wchars_num = MultiByteToWideChar(CP_UTF8, 0, narrow_output, -1, NULL, 0);
    *output = (wchar_t*)malloc(wchars_num * sizeof(wchar_t));
    if (*output) MultiByteToWideChar(CP_UTF8, 0, narrow_output, -1, *output, wchars_num);
    free(narrow_output);

    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return exit_code == 0;
}

// PATH環境変数から実行ファイルを検索
BOOL find_executable_in_path(const wchar_t* exe_name, wchar_t* out_path, size_t out_path_size) {
    return SearchPathW(NULL, exe_name, NULL, (DWORD)out_path_size, out_path, NULL) > 0;
}

// フルパスから親ディレクトリのパスを取得
void get_parent_path(const wchar_t* path, wchar_t* parent_path, size_t parent_path_size) {
    wcsncpy_s(parent_path, parent_path_size, path, _TRUNCATE);
    wchar_t* last_slash = wcsrchr(parent_path, L'\\');
    if (last_slash) *last_slash = L'\0';
}

// パスから拡張子を取得
const wchar_t* get_extension(const wchar_t* path) {
    const wchar_t* dot = wcsrchr(path, L'.');
    return (!dot || dot == path) ? NULL : dot;
}

// パスから拡張子を除いたファイル名（ステム）を取得
void get_stem(const wchar_t* path, wchar_t* stem, size_t stem_size) {
    const wchar_t* last_slash = wcsrchr(path, L'\\');
    const wchar_t* filename = last_slash ? last_slash + 1 : path;
    wcsncpy_s(stem, stem_size, filename, _TRUNCATE);
    wchar_t* last_dot = wcsrchr(stem, L'.');
    if (last_dot) *last_dot = L'\0';
}

// ディレクトリを再帰的に削除
BOOL remove_directory_recursively(const wchar_t* path) {
    wchar_t path_double_null[MAX_PATH + 1] = {0};
    wcsncpy_s(path_double_null, MAX_PATH, path, _TRUNCATE);
    // SHFileOperationW は二重ヌル終端文字列を要求する
    SHFILEOPSTRUCTW file_op = {
        NULL, FO_DELETE, path_double_null, NULL,
        FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
        FALSE, 0, L""
    };
    return SHFileOperationW(&file_op) == 0;
}

// ワイド文字(UTF-16)でファイル内容を読み込む
BOOL read_file_content_wide(const wchar_t* path, wchar_t** content) {
    HANDLE h_file = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (h_file == INVALID_HANDLE_VALUE) return FALSE;

    DWORD file_size = GetFileSize(h_file, NULL);
    if (file_size == INVALID_FILE_SIZE) { CloseHandle(h_file); return FALSE; }

    char* buffer = (char*)malloc(file_size + 1);
    if (!buffer) { CloseHandle(h_file); return FALSE; }

    DWORD bytes_read;
    if (!ReadFile(h_file, buffer, file_size, &bytes_read, NULL) || bytes_read != file_size) {
        free(buffer); CloseHandle(h_file); return FALSE;
    }
    buffer[file_size] = '\0';
    CloseHandle(h_file);

    // BOMチェック (UTF-8 or other encodings)
    int offset = 0;
    UINT code_page = CP_ACP; // Default to ANSI codepage
    if (file_size >= 3 && (unsigned char)buffer[0] == 0xEF && (unsigned char)buffer[1] == 0xBB && (unsigned char)buffer[2] == 0xBF) {
        code_page = CP_UTF8;
        offset = 3;
    } else if (file_size >= 2 && (unsigned char)buffer[0] == 0xFF && (unsigned char)buffer[1] == 0xFE) {
        // UTF-16 LE, just copy memory
        *content = (wchar_t*)malloc(file_size - 1);
        if (*content) {
            memcpy(*content, buffer + 2, file_size - 2);
            (*content)[(file_size / 2) - 1] = L'\0';
        }
        free(buffer);
        return *content != NULL;
    }

    int wide_char_size = MultiByteToWideChar(code_page, 0, buffer + offset, -1, NULL, 0);
    if (wide_char_size == 0) { free(buffer); return FALSE; }

    *content = (wchar_t*)malloc(wide_char_size * sizeof(wchar_t));
    if (!*content) { free(buffer); return FALSE; }

    if (MultiByteToWideChar(code_page, 0, buffer + offset, -1, *content, wide_char_size) == 0) {
        free(*content); free(buffer);
        return FALSE;
    }

    free(buffer);
    return TRUE;
}

// crunの一時ディレクトリを掃除する
void clean_temp_directories(const wchar_t* target_dir) {
    wchar_t search_path[MAX_PATH];
    swprintf_s(search_path, MAX_PATH, L"%s\\crun_tmp_*", target_dir);

    WIN32_FIND_DATAW find_data;
    HANDLE h_find = FindFirstFileW(search_path, &find_data);

    if (h_find == INVALID_HANDLE_VALUE) {
        wprintf(L"No crun temporary directories to clean.\n");
        return;
    }

    int count = 0;
    do {
        if (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            wchar_t dir_to_delete[MAX_PATH];
            swprintf_s(dir_to_delete, MAX_PATH, L"%s\\%s", target_dir, find_data.cFileName);
            wprintf(L"Removing: %s\n", dir_to_delete);
            if (remove_directory_recursively(dir_to_delete)) {
                count++;
            } else {
                fwprintf_err(L"Warning: Failed to remove directory %s\n", dir_to_delete);
            }
        }
    } while (FindNextFileW(h_find, &find_data) != 0);

    FindClose(h_find);

    if (count > 0) {
        wprintf(L"\nSuccessfully removed %d temporary director(y/ies).\n", count);
    } else {
        wprintf(L"No crun temporary directories found to clean.\n");
    }
}
