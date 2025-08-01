## crun コンパイルメモ (v0.7.0)

### 目的
- ClangとGCCの両方に対応
- 実行ファイルサイズを100KB未満に削減
- C++コードの保守性を維持

### ファイルサイズ削減のためのアプローチ
1.  **標準ライブラリの依存削減**: `iostream` や `filesystem` の代わりに、C標準ライブラリ (`stdio.h`, `stdlib.h`, `wchar.h`, `string.h`) とWin32 APIを直接使用。
2.  **最適化フラグの適用**: 各コンパイラの強力なサイズ最適化オプションと、未使用コードの除去オプションを組み合わせ。

### コンパイルコマンドと結果

#### 1. GCC (g++) でのコンパイル
- **コマンド**:
  ```bash
  g++ C:\Users\Owner\Desktop\for_github\crun\src\crun.cpp C:\Users\Owner\Desktop\for_github\crun\crun.res -o C:\Users\Owner\Desktop\for_github\crun\bin\crun_gcc.exe -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wl,--gc-sections,--strip-all -lkernel32 -luser32 -lshell32 -lmsvcrt -static
  ```
- **結果**: `crun_gcc.exe` のファイルサイズは **47,616 バイト (約46.5 KB)**

#### 2. Clang (clang++) でのコンパイル
- **コマンド**:
  ```bash
  clang++ C:\Users\Owner\Desktop\for_github\crun\src\crun.cpp C:\Users\Owner\Desktop\for_github\crun\crun.res -o C:\Users\Owner\Desktop\for_github\crun\bin\crun_clang.exe -Oz -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wl,--gc-sections,--strip-all -lkernel32 -luser32 -lshell32 -lmsvcrt -static
  ```
- **結果**: `crun_clang.exe` のファイルサイズは **19,456 バイト (約19.0 KB)**

### 最終採用
- 両方のコンパイラ (`crun_gcc.exe` と `crun_clang.exe`) でビルドし、保守性を維持します。
- `crun.exe` は `crun_clang.exe` を指すシンボリックリンクまたはコピーとして配置します。

### 補足
- `wmain` から `main` へのエントリーポイント変更と、`CommandLineToArgvW` を使用した引数取得により、リンカの問題を解決し、両コンパイラでの互換性を確保。
- `compiler_exe_name` の生成ロジックを修正し、`g++.exe` および `clang++.exe` を正しく選択できるように改善。