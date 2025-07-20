# 絶対守ること

改行コードはCRLF

# crun コンパイルメモ (v0.7.0)

## 目的

- ClangとGCCの両方に対応
- 実行ファイルサイズを100KB未満に削減
- C++コードの保守性を維持

## コード構成

`crun.cpp`のコードは、機能ごとに以下のファイルに分割され整理されています。

-   **`src/crun.cpp`**: メインのプログラムロジック、コマンドライン引数解析、コンパイルと実行のフローを管理します。
-   **`src/utils.h` / `src/utils.cpp`**: ファイル操作、プロセス実行、パス操作、エラー出力など、汎用的なヘルパー関数が含まれます。
-   **`src/version.h` / `src/version.cpp`**: `crun`のバージョン情報と、その表示に関する関数が含まれます。

## ファイルサイズ削減のためのアプローチ

1.  **標準ライブラリの依存削減**: `iostream` や `filesystem` の代わりに、C標準ライブラリ (`stdio.h`, `stdlib.h`, `wchar.h`, `string.h`) とWin32 APIを直接使用。
2.  **最適化フラグの適用**: 各コンパイラの強力なサイズ最適化オプションと、未使用コードの除去オプションを組み合わせ。

## コンパイルコマンドと結果

### 開発・テスト用ビルド

コードの変更や機能追加のテストを行うためのビルドです。

#### 1. GCC (g++) でのテストビルド

-   **コマンド**:
    ```bash
    g++ src/crun.cpp src/utils.cpp src/version.cpp res/crun.res -o bin/crun_test.exe -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wl,--gc-sections,--strip-all -lkernel32 -luser32 -lshell32 -lmsvcrt -static
    ```
-   **出力**: `bin/crun_test.exe`

#### 2. Clang (clang++) でのテストビルド

-   **コマンド**:
    ```bash
    clang++ src/crun.cpp src/utils.cpp src/version.cpp res/crun.res -o bin/crun_test_clang.exe -Oz -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wl,--gc-sections,--strip-all -lkernel32 -luser32 -lshell32 -lmsvcrt -static
    ```
-   **出力**: `bin/crun_test_clang.exe`

### リリース用ビルド

最終的な配布物として、最小サイズに最適化されたビルドです。`crun.rc`のバージョン情報もこの段階で更新されます。

#### 1. GCC (g++) でのリリースビルド

-   **コマンド**:
    ```bash
    g++ src/crun.cpp src/utils.cpp src/version.cpp res/crun.res -o bin/crun_gcc.exe -Os -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wl,--gc-sections,--strip-all -lkernel32 -luser32 -lshell32 -lmsvcrt -static
    ```
-   **出力**: `bin/crun_gcc.exe`

#### 2. Clang (clang++) でのリリースビルド

-   **コマンド**:
    ```bash
    clang++ src/crun.cpp src/utils.cpp src/version.cpp res/crun.res -o bin/crun_clang.exe -Oz -s -fno-exceptions -fno-rtti -ffunction-sections -fdata-sections -Wl,--gc-sections,--strip-all -lkernel32 -luser32 -lshell32 -lmsvcrt -static
    ```
-   **出力**: `bin/crun_clang.exe`

**最終的な `crun.exe` の配置**:
`crun.exe` は通常、`bin/crun_clang.exe` のシンボリックリンクまたはコピーとして配置されます。

## 最終採用

- 両方のコンパイラ (`crun_gcc.exe` と `crun_clang.exe`) でビルドし、保守性を維持します。
- `crun.exe` は `crun_clang.exe` を指すシンボリックリンクまたはコピーとして配置します。

## 補足

- `wmain` から `main` へのエントリーポイント変更と、`CommandLineToArgvW` を使用した引数取得により、リンカの問題を解決し、両コンパイラでの互換性を確保。
- `compiler_exe_name` の生成ロジックを修正し、`g++.exe` および `clang++.exe` を正しく選択できるように改善。