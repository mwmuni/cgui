#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include <glad/glad.h>
#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>

// Define button identifiers
#define ID_BUTTON_OK 1
#define ID_BUTTON_QUIT 2
#define ID_BUTTON_GENERATE 3

#define IMG_WIDTH 100
#define IMG_WIDTH_2 50
#define IMG_HEIGHT 100
#define IMG_HEIGHT_2 50

// Define the center of the crescent
#define centerX IMG_WIDTH_2
#define centerY IMG_HEIGHT_2

// Define the radius of the crescent
#define radius 20
#define leftLimit (centerX - radius)
#define rightLimit (centerX + radius)
#define topLimit (centerY - radius)
#define bottomLimit centerY

#define width (rightLimit - leftLimit + 1)

// Custom window procedure declaration
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Global variables for image rendering
unsigned char image[100][100][3];
HBITMAP bitmap;
HDC memDC;
BITMAPINFO bmi;

void generateRandomImage() {
    for (int i = 0; i < IMG_HEIGHT; ++i) { // Height
        for (int j = 0; j < IMG_WIDTH; ++j) { // Width
            image[i][j][0] = rand() % 256; // Red
            image[i][j][1] = rand() % 256; // Green
            image[i][j][2] = rand() % 256; // Blue
        }
    }
}

void generateCrescentImage() {
    for (int i = 0; i < IMG_HEIGHT; ++i) {
        for (int j = 0; j < IMG_WIDTH; ++j) {
            image[i][j][0] = 0; // Red
            image[i][j][1] = 0; // Green
            image[i][j][2] = 0; // Blue
        }
    }

    // Generate a crescent shape using two sine functions

    // Define the color of the crescent
    const unsigned char color[3] = {255, 255, 255};

    // Generate the crescent shape

    const int crescentStart[2] = {centerX - radius, centerY};
    const int crescentEnd[2] = {centerX + radius, centerY};

    // We want 0 to pi to define the region of the sine function to use
    // We will define an upper and lower bound for the region by using two sine functions
    // The upper bound will simply be the first sine function * 1.2
    // The lower bound will be the first sine function

    // The highest point of the crescent will be at pi/2 for the upper bound
    // The lowest point of the crescent will be at 0 and pi for both the upper and lower bounds

    float xMap[width];

    const float pi_step = M_PI / (rightLimit - leftLimit);

    // Precompute the X values
    for (int i = leftLimit; i <= rightLimit; ++i) {
        xMap[i - leftLimit] = pi_step * (i-leftLimit);
    }

    // Precompute the Y values for the upper bound
    float upperBound[rightLimit - leftLimit + 1];
    for (int i = leftLimit; i <= rightLimit; ++i) {
        upperBound[i - leftLimit] = centerY - radius * sin(xMap[i - leftLimit]) * 0.8;
    }

    // Precompute the Y values for the lower bound
    float lowerBound[rightLimit - leftLimit + 1];
    for (int i = leftLimit; i <= rightLimit; ++i) {
        lowerBound[i - leftLimit] = centerY - radius * sin(xMap[i - leftLimit]);
    }

    for (int i = leftLimit; i <= rightLimit; ++i) {
        for (int j = topLimit; j <= bottomLimit; ++j) {
            // We need to determine if the current pixel is within the crescent shape
            // We can do this by checking if the current pixel is within the bounds of the crescent

            // Check if the current pixel is within the bounds of the crescent
            if (j >= lowerBound[i - leftLimit] && j <= upperBound[i - leftLimit])
            {
                image[j][i][0] = color[0];
                image[j][i][1] = color[1];
                image[j][i][2] = color[2];
            }
        }
    }

    // Make the start and end points of the crescent black
    image[crescentStart[1]][crescentStart[0]][0] = 0;
    image[crescentStart[1]][crescentStart[0]][1] = 0;
    image[crescentStart[1]][crescentStart[0]][2] = 0;

    image[crescentEnd[1]][crescentEnd[0]][0] = 0;
    image[crescentEnd[1]][crescentEnd[0]][1] = 0;
    image[crescentEnd[1]][crescentEnd[0]][2] = 0;
}

void addIris() {
    // Define the center of the iris
    const int irisCenter[2] = {centerX, centerY};

    // Define the radius of the iris
    const int irisRadius = 10;

    // Define the color of the iris
    const unsigned char color[3] = {255, 255, 255};

    // Generate the iris shape
    for (int i = 0; i < IMG_HEIGHT; ++i) {
        for (int j = 0; j < IMG_WIDTH; ++j) {
            // We need to determine if the current pixel is within the iris shape
            // We can do this by checking if the current pixel is within the bounds of the iris

            // Check if the current pixel is within the bounds of the iris
            if ((i - irisCenter[1]) * (i - irisCenter[1]) + (j - irisCenter[0]) * (j - irisCenter[0]) <= irisRadius * irisRadius)
            {
                image[i][j][0] = color[0];
                image[i][j][1] = color[1];
                image[i][j][2] = color[2];
            }
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
                    if (rand() % 100 != 0) {
                        generateRandomImage();
                    } else {
                        generateCrescentImage();
                        addIris();
                    }
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
