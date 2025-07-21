$files = @(
"C:\Users\Owner\Desktop\for_github\crun\src\compiler.cpp",
"C:\Users\Owner\Desktop\for_github\crun\src\options.cpp",
"C:\Users\Owner\Desktop\for_github\crun\src\options.h",
"C:\Users\Owner\Desktop\for_github\crun\src\compiler.h",
"C:\Users\Owner\Desktop\for_github\crun\src\utils.h",
"C:\Users\Owner\Desktop\for_github\crun\src\utils.cpp",
"C:\Users\Owner\Desktop\for_github\crun\src\crun.cpp",
"C:\Users\Owner\Desktop\for_github\crun\test\features\rc_test.rc",
"C:\Users\Owner\Desktop\for_github\crun\test\features\rc_test.c",
"C:\Users\Owner\Desktop\for_github\crun\.gemini\GEMINI.md",
"C:\Users\Owner\Desktop\for_github\crun\.vscode\settings.json",
"C:\Users\Owner\Desktop\for_github\crun\crun_improvements.md",
"C:\Users\Owner\Desktop\for_github\crun\DEVELOPMENT.md",
"C:\Users\Owner\Desktop\for_github\crun\memo.txt",
"C:\Users\Owner\Desktop\for_github\crun\README.md",
"C:\Users\Owner\Desktop\for_github\crun\res\crun.rc",
"C:\Users\Owner\Desktop\for_github\crun\src\version.cpp",
"C:\Users\Owner\Desktop\for_github\crun\src\version.h",
"C:\Users\Owner\Desktop\for_github\crun\test\basic\args.c",
"C:\Users\Owner\Desktop\for_github\crun\test\basic\hello.c",
"C:\Users\Owner\Desktop\for_github\crun\test\basic\hello.cpp",
"C:\Users\Owner\Desktop\for_github\crun\test\basic\helper.c",
"C:\Users\Owner\Desktop\for_github\crun\test\basic\helper.h",
"C:\Users\Owner\Desktop\for_github\crun\test\basic\main.c",
"C:\Users\Owner\Desktop\for_github\crun\test\basic\single_file_test.c",
"C:\Users\Owner\Desktop\for_github\crun\test\basic\single_file_test.cpp",
"C:\Users\Owner\Desktop\for_github\crun\test\errors\compile_error.c",
"C:\Users\Owner\Desktop\for_github\crun\test\errors\warning.c",
"C:\Users\Owner\Desktop\for_github\crun\test\features\direct2d_test.cpp",
"C:\Users\Owner\Desktop\for_github\crun\test\features\libs_option_test.c",
"C:\Users\Owner\Desktop\for_github\crun\test\features\math_test.c",
"C:\Users\Owner\Desktop\for_github\crun\test\features\pragma_comment_test.c",
"C:\Users\Owner\Desktop\for_github\crun\test\features\pragma_comment_test.h",
"C:\Users\Owner\Desktop\for_github\crun\test\features\pthread_test.c",
"C:\Users\Owner\Desktop\for_github\crun\test\features\verify_lean.c",
"C:\Users\Owner\Desktop\for_github\crun\test\features\verify_pragma.c",
"C:\Users\Owner\Desktop\for_github\crun\test\performance\prime_load_test.c"
)

foreach ($file in $files) {
    Write-Host "Converting $file to CRLF..."
    (Get-Content $file -Raw) | Set-Content $file -NoNewline -Encoding Default
}
Write-Host "All files converted to CRLF."
