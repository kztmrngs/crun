#pragma once

#include "options.h"
#include <windows.h>

// --- Function Declarations ---
// --- 関数宣言 ---
BOOL find_compiler(const wchar_t* compiler_name, BOOL has_cpp, wchar_t* compiler_path, size_t path_size);
BOOL build_compile_command(const ProgramOptions* opts, const wchar_t* executable_path, const wchar_t* compiler_path, BOOL has_cpp, wchar_t* command, size_t command_size);
