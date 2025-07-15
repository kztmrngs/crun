#include <windows.h>
#include <cryptuiapi.h>
#include <stdio.h>

#pragma comment(lib, "cryptui.lib")

int main() {
    // This function is just an example and won't actually show a dialog.
    // We're just checking if it can be linked correctly.
    if (CryptUIDlgSelectCertificateW != NULL) {
        printf("CryptUIDlgSelectCertificateW is available.\n");
    } else {
        printf("CryptUIDlgSelectCertificateW is not available.\n");
    }
    return 0;
}

