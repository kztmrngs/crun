@echo off
echo Setting Git core.autocrlf to true...
git config core.autocrlf true

echo Normalizing line endings to CRLF for all files...
git add --renormalize .

echo Line ending normalization complete.
pause