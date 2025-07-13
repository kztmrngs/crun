#ifndef CRUN_UTILS_H
#define CRUN_UTILS_H

#include <windows.h>
#include <stdio.h>
#include <wchar.h>

#ifdef __cplusplus
extern "C" {
#endif

void fwprintf_err(const wchar_t* format, ...);
BOOL file_exists(const wchar_t* path);
BOOL run_process(wchar_t* command_line, BOOL verbose);
BOOL run_program_and_get_exit_code(wchar_t* command_line, DWORD* p_exit_code);
BOOL run_process_and_capture_output(wchar_t* command_line, wchar_t** output);
BOOL find_executable_in_path(const wchar_t* exe_name, wchar_t* out_path, size_t out_path_size);
void get_parent_path(const wchar_t* path, wchar_t* parent_path, size_t parent_path_size);
const wchar_t* get_extension(const wchar_t* path);
void get_stem(const wchar_t* path, wchar_t* stem, size_t stem_size);
BOOL remove_directory_recursively(const wchar_t* path);
BOOL read_file_content_wide(const wchar_t* path, wchar_t** content);
void clean_temp_directories(const wchar_t* target_dir);

#ifdef __cplusplus
}
#endif

#endif // CRUN_UTILS_H
