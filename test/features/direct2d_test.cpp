// direct2d_test.cpp
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <stdio.h>

// Link with necessary libraries
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

int main() {
    ID2D1Factory* pD2DFactory = NULL;
    IDWriteFactory* pDWriteFactory = NULL;

    HRESULT hr = D2D1CreateFactory(
        D2D1_FACTORY_TYPE_SINGLE_THREADED,
        &pD2DFactory
    );

    if (SUCCEEDED(hr)) {
        printf("Direct2D Factory created successfully.\n");
    } else {
        printf("Failed to create Direct2D Factory. HRESULT: 0x%08X\n", hr);
        return 1;
    }

    hr = DWriteCreateFactory(
        DWRITE_FACTORY_TYPE_SHARED,
        __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&pDWriteFactory)
    );

    if (SUCCEEDED(hr)) {
        printf("DirectWrite Factory created successfully.\n");
    } else {
        printf("Failed to create DirectWrite Factory. HRESULT: 0x%08X\n", hr);
        if (pD2DFactory) pD2DFactory->Release();
        return 1;
    }

    printf("Test passed: Direct2D and DirectWrite factories were created.\n");

    if (pDWriteFactory) pDWriteFactory->Release();
    if (pD2DFactory) pD2DFactory->Release();

    return 0;
}