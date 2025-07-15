#include "compiler.h"
#include "utils.h"
#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <shlwapi.h> // For PathCanonicalizeW

// --- データ構造 ---
// ヘッダーファイルと対応するライブラリをマッピングする構造体
struct HeaderToLib {
    const wchar_t* header;
    const wchar_t* library;
};

// Win32 APIおよびその他の一般的なライブラリのマッピングテーブル
static const HeaderToLib lib_map[] = {
    // コアWindowsライブラリ (Core Windows Libraries)
    {L"windows.h", L"-lkernel32 -luser32 -lgdi32 -lwinspool -lcomdlg32 -ladvapi32 -lshell32 -lole32 -loleaut32 -luuid"},
    {L"winbase.h", L"-lkernel32"}, {L"winnt.h", L"-lkernel32"}, {L"libloaderapi.h", L"-lkernel32"},
    {L"winuser.h", L"-luser32"}, {L"wingdi.h", L"-lgdi32"}, {L"winreg.h", L"-ladvapi32"},
    {L"winsvc.h", L"-ladvapi32"}, {L"advapi32.h", L"-ladvapi32"}, {L"cfgmgr32.h", L"-lcfgmgr32"},
    {L"devguid.h", L"-ldevguid"}, {L"devobj.h", L"-ldevobj"}, {L"userenv.h", L"-luserenv"},
    {L"wer.h", L"-lwer"}, {L"winspool.h", L"-lwinspool"}, {L"version.h", L"-lversion"},

    // シェル (Shell)
    {L"shellapi.h", L"-lshell32"}, {L"shlobj.h", L"-lole32 -lshell32"}, {L"shcore.h", L"-lshcore"},
    {L"shlwapi.h", L"-lshlwapi"}, {L"propsys.h", L"-lpropsys"},
    {L"shellscalingapi.h", L"-lshellscalingapi"},

    // COM, OLE, ActiveX
    {L"ole32.h", L"-lole32"}, {L"oleauto.h", L"-loleaut32"}, {L"combaseapi.h", L"-lole32"},
    {L"comdlg32.h", L"-lcomdlg32"}, {L"commdlg.h", L"-lcomdlg32"}, {L"comctl32.h", L"-lcomctl32"},
    {L"commctrl.h", L"-lcomctl32"}, {L"urlmon.h", L"-lurlmon"}, {L"uuid.h", L"-luuid"},
    {L"ocidl.h", L"-lole32"}, {L"olectl.h", L"-lole32"},

    // ネットワーク (Networking)
    {L"winsock2.h", L"-lws2_32"}, {L"ws2tcpip.h", L"-lws2_32"}, {L"winsock.h", L"-lws2_32"},
    {L"wininet.h", L"-lwininet"}, {L"winhttp.h", L"-lwinhttp"}, {L"iphlpapi.h", L"-liphlpapi"},
    {L"dnsapi.h", L"-ldnsapi"}, {L"dhcpcsvc.h", L"-ldhcpcsvc"}, {L"mpr.h", L"-lmpr"},
    {L"netapi32.h", L"-lnetapi32"}, {L"http.h", L"-lhttpapi"},

    // グラフィックスとマルチメディア (Graphics and Multimedia)
    // DirectX
    {L"d3d9.h", L"-ld3d9"}, {L"d3dx9.h", L"-ld3dx9"}, {L"d3d10.h", L"-ld3d10"},
    {L"d3d11.h", L"-ld3d11"}, {L"d3d12.h", L"-ld3d12"}, {L"d3dcompiler.h", L"-ld3dcompiler"},
    {L"d2d1.h", L"-ld2d1"}, {L"dwrite.h", L"-ldwrite"}, {L"dxgi.h", L"-ldxgi"},
    {L"dinput8.h", L"-ldinput8"}, {L"dsound.h", L"-ldsound"}, {L"xaudio2.h", L"-lxaudio2"},
    {L"xinput.h", L"-lxinput"},
    // OpenGL
    {L"gl/gl.h", L"-lopengl32"}, {L"gl/glu.h", L"-lglu32"}, {L"gl/glaux.h", L"-lglaux"},
    {L"opengl.h", L"-lopengl32"}, {L"glu.h", L"-lglu32"},
    // Windows GDI & UI
    {L"gdiplus.h", L"-lgdiplus"}, {L"dwmapi.h", L"-ldwmapi"}, {L"uxtheme.h", L"-luxtheme"},
    {L"imm32.h", L"-limm32"}, {L"dcomp.h", L"-ldcomp"},
    // Windows Media Foundation & Audio
    {L"mf.h", L"-lmf"}, {L"mfplat.h", L"-lmfplat"}, {L"mfreadwrite.h", L"-lmfreadwrite"},
    {L"mfuuid.h", L"-lmfuuid"}, {L"avrt.h", L"-lavrt"}, {L"vfw.h", L"-lvfw32"},
    {L"winmm.h", L"-lwinmm"},

    // セキュリティ (Security)
    {L"rpc.h", L"-lrpcrt4"}, {L"bcrypt.h", L"-lbcrypt"}, {L"ncrypt.h", L"-lncrypt"},
    {L"setupapi.h", L"-lsetupapi"}, {L"wintrust.h", L"-lwintrust"}, {L"imagehlp.h", L"-limagehlp"},
    {L"psapi.h", L"-lpsapi"}, {L"cryptuiapi.h", L"-lcryptui"}, {L"wincrypt.h", L"-lcrypt32"},
    {L"secur32.h", L"-lsecur32"}, {L"sspi.h", L"-lsecur32"},

    // データアクセス (Data Access)
    {L"odbcinst.h", L"-lodbccp32"}, {L"sqlext.h", L"-lodbc32"}, {L"sql.h", L"-lodbc32"},
    {L"oledb.h", L"-loledb"},

    // デバイスとプリンティング (Device and Printing)
    {L"hidsdi.h", L"-lhid"}, {L"winusb.h", L"-lwinusb"}, {L"bluetoothapis.h", L"-lbthprops"},
    {L"spoolss.h", L"-lwinspool"},

    // 管理とWMI (Management and WMI)
    {L"wbemidl.h", L"-lwbemuuid"}, {L"wmistr.h", L"-lwbemuuid"},

    // その他 (Miscellaneous)
    {L"msi.h", L"-lmsi"}, {L"powrprof.h", L"-lpowrprof"}, {L"wtsapi32.h", L"-lwtsapi32"},
    {L"virtdisk.h", L"-lvirtdisk"},

    // 標準ライブラリ (Standard Libraries - MinGW-specific)
    {L"pthread.h", L"-lpthread"}, {L"math.h", L"-lm"},
};

// --- ヘルパー関数 ---
// 指定された行で#includeまたは#pragmaがコメントアウトされているかチェックする
bool is_commented_out(const wchar_t* line_start, const wchar_t* directive_pos) {
    const wchar_t* p = line_start;
    while (p < directive_pos) {
        if (*p == L'/' && *(p + 1) == L'/') {
            return true; // //行コメントの内側
        }
        p++;
    }
    return false;
}

// 再帰的スキャン関数の前方宣言
void scan_file_for_libs_recursive(const wchar_t* file_path, wchar_t* auto_flags, size_t auto_flags_size, wchar_t* linked_libs, size_t linked_libs_size, wchar_t** processed_files, int* processed_count, int max_processed);

// 全てのソースファイルにわたってライブラリ検索を調整するメイン関数
void find_libs_in_sources(const ProgramOptions* opts, wchar_t* auto_flags, size_t auto_flags_size) {
    wchar_t linked_libs[1024] = {0}; // 重複を避けるためにリンクされたライブラリを追跡
    wchar_t* processed_files[256] = {0}; // 無限再帰を避けるために処理されたファイルを追跡
    int processed_count = 0;

    for (int i = 0; i < opts->num_source_files; ++i) {
        wchar_t full_path[MAX_PATH];
        if (GetFullPathNameW(opts->source_files[i], MAX_PATH, full_path, NULL)) {
            scan_file_for_libs_recursive(full_path, auto_flags, auto_flags_size, linked_libs, _countof(linked_libs), processed_files, &processed_count, _countof(processed_files));
        }
    }

    // 処理済みファイルパスの追跡用に割り当てられたメモリを解放
    for (int i = 0; i < processed_count; ++i) {
        free(processed_files[i]);
    }
}

// #includeおよび#pragmaディレクティブをファイルで再帰的にスキャンしてライブラリフラグを構築する
void scan_file_for_libs_recursive(const wchar_t* file_path, wchar_t* auto_flags, size_t auto_flags_size, wchar_t* linked_libs, size_t linked_libs_size, wchar_t** processed_files, int* processed_count, int max_processed) {
    // 1. 同じファイルを複数回処理しないようにする
    for (int i = 0; i < *processed_count; ++i) {
        if (wcscmp(processed_files[i], file_path) == 0) return;
    }

    // 2. 処理済みファイルのリストにファイルを追加する
    if (*processed_count >= max_processed) return; // 深い再帰のための安全停止
    processed_files[*processed_count] = _wcsdup(file_path);
    (*processed_count)++;

    // 3. ファイルの内容を読み込む
    wchar_t* content = NULL;
    if (!read_file_content_wide(file_path, &content)) return;

    // 4. 内容を行ごとにスキャンする
    const wchar_t* line = content;
    while (line && *line != L'\0') {
        const wchar_t* line_end = wcschr(line, L'\n');
        
        // a. #include <...> と #include "..." をチェックする
        for (size_t j = 0; j < _countof(lib_map); ++j) {
            const wchar_t* header_pos = wcsstr(line, lib_map[j].header);
            if (header_pos && (!line_end || header_pos < line_end) && !is_commented_out(line, header_pos)) {
                if (!wcsstr(linked_libs, lib_map[j].library)) {
                    wcscat_s(auto_flags, auto_flags_size, L" ");
                    wcscat_s(auto_flags, auto_flags_size, lib_map[j].library);
                    wcscat_s(linked_libs, linked_libs_size, lib_map[j].library);
                    wcscat_s(linked_libs, linked_libs_size, L" ");
                }
            }
        }

        // b. #pragma comment(lib, "...") をチェックする
        const wchar_t* pragma_pos = wcsstr(line, L"#pragma comment(lib,");
        if (pragma_pos && (!line_end || pragma_pos < line_end) && !is_commented_out(line, pragma_pos)) {
            const wchar_t* lib_start = wcschr(pragma_pos, L'"');
            if (lib_start) {
                lib_start++; // 開始引用符を過ぎて移動
                const wchar_t* lib_end = wcschr(lib_start, L'"');
                if (lib_end) {
                    wchar_t lib_name[256] = {0};
                    wcsncpy_s(lib_name, _countof(lib_name), lib_start, lib_end - lib_start);
                    
                    // -lフラグとの互換性のために.libサフィックスがあれば削除
                    wchar_t* dot_lib = wcsstr(lib_name, L".lib");
                    if (dot_lib) *dot_lib = L'\0';

                    wchar_t lib_flag[260];
                    swprintf_s(lib_flag, _countof(lib_flag), L"-l%s", lib_name);

                    if (!wcsstr(linked_libs, lib_flag)) {
                        wcscat_s(auto_flags, auto_flags_size, L" ");
                        wcscat_s(auto_flags, auto_flags_size, lib_flag);
                        wcscat_s(linked_libs, linked_libs_size, lib_flag);
                        wcscat_s(linked_libs, linked_libs_size, L" ");
                    }
                }
            }
        }

        // c. #include "relative_path.h" をチェックして再帰する
        const wchar_t* include_pos = wcsstr(line, L"#include \"");
        if (include_pos && (!line_end || include_pos < line_end) && !is_commented_out(line, include_pos)) {
            const wchar_t* path_start = include_pos + wcslen(L"#include \"");
            const wchar_t* path_end = wcschr(path_start, L'"');
            if (path_end) {
                wchar_t rel_path[MAX_PATH];
                wcsncpy_s(rel_path, _countof(rel_path), path_start, path_end - path_start);

                wchar_t current_dir[MAX_PATH] = {0};
                wcsncpy_s(current_dir, _countof(current_dir), file_path, (wcsrchr(file_path, L'\\') - file_path + 1));
                
                wchar_t header_full_path[MAX_PATH];
                swprintf_s(header_full_path, _countof(header_full_path), L"%s%s", current_dir, rel_path);
                
                wchar_t canonical_path[MAX_PATH];
                if (PathCanonicalizeW(canonical_path, header_full_path)) {
                    scan_file_for_libs_recursive(canonical_path, auto_flags, auto_flags_size, linked_libs, linked_libs_size, processed_files, processed_count, max_processed);
                }
            }
        }

        line = line_end ? (line_end + 1) : NULL;
    }
    free(content);
}


// --- コンパイラ設定 ---
BOOL find_compiler(const wchar_t* compiler_name, BOOL has_cpp, wchar_t* compiler_path, size_t path_size) {
    wchar_t compiler_exe_name[20];
    if (has_cpp) {
        wcscpy_s(compiler_exe_name, 20, (wcscmp(compiler_name, L"gcc") == 0) ? L"g++.exe" : L"clang++.exe");
    } else {
        wcscpy_s(compiler_exe_name, 20, (wcscmp(compiler_name, L"gcc") == 0) ? L"gcc.exe" : L"clang.exe");
    }

    if (!find_executable_in_path(compiler_exe_name, compiler_path, path_size)) {
        fwprintf_err(L"エラー: コンパイラ '%s' がPATHに見つかりません。\n"
                     L"MinGW (gcc/g++) または Clang がインストールされ、その 'bin' ディレクトリがシステムのPATH環境変数に追加されていることを確認してください。\n", compiler_exe_name);
        return FALSE;
    }
    return TRUE;
}

// --- コンパイル ---
BOOL build_compile_command(const ProgramOptions* opts, const wchar_t* executable_path, const wchar_t* compiler_path, BOOL has_cpp, wchar_t* command, size_t command_size) {
    wchar_t all_source_files_str[32767] = {0};
    for (int i = 0; i < opts->num_source_files; ++i) {
        wchar_t full_path[MAX_PATH];
        if (!GetFullPathNameW(opts->source_files[i], MAX_PATH, full_path, NULL)) {
            fwprintf_err(L"エラー: ソースファイルのフルパスを取得できませんでした: %s\n", opts->source_files[i]);
            return FALSE;
        }
        wcscat_s(all_source_files_str, _countof(all_source_files_str), L" \"");
        wcscat_s(all_source_files_str, _countof(all_source_files_str), full_path);
        wcscat_s(all_source_files_str, _countof(all_source_files_str), L"\"");
    }

    wchar_t auto_flags[2048] = {0}; // より多くのライブラリフラグのためにサイズを増加
    if (opts->debug_build) {
        wcscpy_s(auto_flags, _countof(auto_flags), L"-g");
    } else {
        wcscpy_s(auto_flags, _countof(auto_flags), L"-O2 -s");
    }

    // ソースファイルとヘッダーファイルをスキャンして必要なライブラリをすべて見つける
    find_libs_in_sources(opts, auto_flags, _countof(auto_flags));

    if (opts->warnings_all) {
        wcscat_s(auto_flags, _countof(auto_flags), L" -Wall");
    }

    swprintf_s(command, command_size, L"%s %s %s %s -o \"%s\"",
               compiler_path,
               all_source_files_str,
               auto_flags,
               opts->compiler_flags,
               executable_path);

    return TRUE;
}