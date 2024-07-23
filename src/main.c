#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <glad/glad.h>
#include <windows.h>

// Define button identifiers
#define ID_BUTTON_OK 1
#define ID_BUTTON_QUIT 2
#define ID_BUTTON_GENERATE 3

// Custom window procedure declaration
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Global variables for image rendering
unsigned char image[100][100][3];
HBITMAP bitmap;
HDC memDC;
BITMAPINFO bmi;

void generateRandomImage() {
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 100; ++j) {
            image[i][j][0] = rand() % 256; // Red
            image[i][j][1] = rand() % 256; // Green
            image[i][j][2] = rand() % 256; // Blue
        }
    }
}

void renderImage(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    SetDIBits(memDC, bitmap, 0, 100, image, &bmi, DIB_RGB_COLORS);
    BitBlt(hdc, 250, 250, 100, 100, memDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

// Modify createWindow to use custom window procedure and set button IDs
void createWindow(void) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc; // Use custom window procedure
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "Conan";
    RegisterClass(&wc);

    HWND window = CreateWindow(wc.lpszClassName, "Conan", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 800, 600, 0, 0, wc.hInstance, 0);
    CreateWindow("BUTTON", "OK", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 10, 100, 30, window, (HMENU)ID_BUTTON_OK, wc.hInstance, 0);
    CreateWindow("EDIT", "Hello, World!", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE, 10, 50, 200, 100, window, 0, wc.hInstance, 0);
    CreateWindow("BUTTON", "Quit", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 160, 100, 30, window, (HMENU)ID_BUTTON_QUIT, wc.hInstance, 0);

    // Create a button to generate the random colors
    CreateWindow("BUTTON", "Generate", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 200, 100, 30, window, (HMENU)ID_BUTTON_GENERATE, wc.hInstance, 0);

    // Initialize image rendering
    srand(time(NULL)); // Seed the random number generator
    generateRandomImage();

    HDC hdc = GetDC(window);
    memDC = CreateCompatibleDC(hdc);
    bitmap = CreateCompatibleBitmap(hdc, 100, 100);
    SelectObject(memDC, bitmap);

    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = 100;
    bmi.bmiHeader.biHeight = -100; // negative to indicate top-down bitmap
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    renderImage(window);
    ReleaseDC(window, hdc);

    // Corrected loop to process all window messages
    while (1) {
        MSG msg;
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(1);
    }

    // Cleanup
    DeleteObject(bitmap);
    DeleteDC(memDC);
}

// Implement the custom window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BUTTON_OK:
                    // Handle OK button click
                    MessageBox(hwnd, "OK Button Clicked", "Button Click", MB_OK);
                    break;
                case ID_BUTTON_QUIT:
                    // Handle Quit button click
                    PostQuitMessage(0);
                    break;
                case ID_BUTTON_GENERATE:
                    // Handle Generate button click
                    generateRandomImage();
                    renderImage(hwnd);
                    break;
            }
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int main(void) {
    createWindow();
    return EXIT_SUCCESS;
}
