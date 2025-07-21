#include <windows.h>
#include <stdio.h>

#define IDS_TEST_STRING 101

int main() {
    wchar_t buffer[256];
    if (LoadStringW(GetModuleHandle(NULL), IDS_TEST_STRING, buffer, sizeof(buffer) / sizeof(buffer[0]))) {
        wprintf(L"Loaded string: %s\n", buffer);
        return 0;
    } else {
        wprintf(L"Failed to load string from resources.\n");
        return 1;
    }
}
