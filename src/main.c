#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <glad/glad.h>
#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <tinycthread.h>
#include <immintrin.h>

#define ID_BUTTON_OK 1
#define ID_BUTTON_QUIT 2
#define ID_BUTTON_GENERATE 3
#define ID_BUTTON_TOGGLE_BLOCK_THREAD 4
#define ID_BUTTON_TOGGLE_GEN_TYPE 5

#define IMG_WIDTH 100
#define IMG_WIDTH_2 50
#define IMG_HEIGHT 100
#define IMG_HEIGHT_2 50
#define BLOCK_WIDTH 100
#define BLOCK_HEIGHT 100
#define BATCH_SIZE 1000

#define centerX IMG_WIDTH_2
#define centerY IMG_HEIGHT_2
#define radius 20
#define leftLimit (centerX - radius)
#define rightLimit (centerX + radius)
#define topLimit (centerY - radius)
#define bottomLimit centerY
#define width (rightLimit - leftLimit + 1)

#define GENERATION_MODE_ALL 0
#define GENERATION_MODE_BATCH 1

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

unsigned char image[100][100][3];
HBITMAP bitmap;
HDC memDC;
BITMAPINFO bmi;

struct thread_data {
    int* keepThreadAlive;
    int* generationMode;
    HWND hwnd;
};

struct mode_thread_data {
    int* generationMode;
    HWND hwnd;
};

int* keepThreadAlive = NULL;
int* generationMode = NULL;

void renderImage(HWND hwnd) {
    HDC hdc = GetDC(hwnd);
    SetDIBits(memDC, bitmap, 0, IMG_HEIGHT, image, &bmi, DIB_RGB_COLORS);
    BitBlt(hdc, 250, 250, IMG_WIDTH, IMG_HEIGHT, memDC, 0, 0, SRCCOPY);
    ReleaseDC(hwnd, hdc);
}

void generateRandomImage() {
    for (int i = 0; i < IMG_HEIGHT; ++i) {
        for (int j = 0; j < IMG_WIDTH; ++j) {
            image[i][j][0] = rand() % 256;
            image[i][j][1] = rand() % 256;
            image[i][j][2] = rand() % 256;
        }
    }
}

void setPixel(int x, int y, unsigned char r, unsigned char g, unsigned char b) {
    if (x >= 0 && x < IMG_WIDTH && y >= 0 && y < IMG_HEIGHT) {
        image[y][x][0] = r;
        image[y][x][1] = g;
        image[y][x][2] = b;
    }
}


int generateRandomBlock(void* arg) {
    struct thread_data* data = (struct thread_data*)arg;
    int* keepThreadAlive = data->keepThreadAlive;
    int* generationMode = data->generationMode;
    
    
    while (*keepThreadAlive) {
        if (*generationMode == GENERATION_MODE_BATCH) {
            for (int i = 0; i < BATCH_SIZE; ++i) {
                int x = rand() % BLOCK_WIDTH;
                int y = rand() % BLOCK_HEIGHT;
                unsigned char r = rand() % 256;
                unsigned char g = rand() % 256;
                unsigned char b = rand() % 256;
                setPixel(x, y, r, g, b);
            }
        } else {
            __m256i random_values;
            __m256i zero = _mm256_setzero_si256();
            __m256i max_val = _mm256_set1_epi32(255);
            int rand_buffer[8];
            
            for (int i = 0; i < BLOCK_WIDTH; ++i) {
                for (int j = 0; j < BLOCK_HEIGHT; j += 8) {
                    for (int k = 0; k < 3; ++k) {
                        random_values = _mm256_set_epi32(
                            rand() % 256, rand() % 256, rand() % 256, rand() % 256,
                            rand() % 256, rand() % 256, rand() % 256, rand() % 256
                        );
                        random_values = _mm256_max_epu8(random_values, zero);
                        random_values = _mm256_min_epu8(random_values, max_val);
                        
                        _mm256_storeu_si256((__m256i*)rand_buffer, random_values);
                        
                        for (int l = 0; l < 8; ++l) {
                            int r = rand_buffer[l];
                            int g = rand_buffer[(l + 1) % 8];
                            int b = rand_buffer[(l + 2) % 8];
                            setPixel(i, j + l, r, g, b);
                        }
                    }
                }
            }
        }
        renderImage(data->hwnd);
        Sleep(1);
    }
    return 0;
}

int manageModeLabelText(void* arg) {
    struct mode_thread_data* data = (struct mode_thread_data*)arg;
    int* generationMode = data->generationMode;
    HWND hwnd = data->hwnd;
    int lastMode = *generationMode;
    while (1) {
        if (lastMode != *generationMode) {
            lastMode = *generationMode;
            const char* modeText = (*generationMode == GENERATION_MODE_ALL) ? "Generation Mode: All" : "Generation Mode: Batch";
            SetWindowText(hwnd, modeText);
        }
        Sleep(100);
    }
    return 0;
}

void generateRandomBlockThreaded(struct thread_data* data) {
    thrd_t thread;
    thrd_create(&thread, generateRandomBlock, data);
    thrd_detach(thread); // Ensure the thread will clean up on its own
}

void generateCrescentImage() {
    for (int i = 0; i < IMG_HEIGHT; ++i) {
        for (int j = 0; j < IMG_WIDTH; ++j) {
            image[i][j][0] = 0;
            image[i][j][1] = 0;
            image[i][j][2] = 0;
        }
    }

    const unsigned char color[3] = {255, 255, 255};
    const int crescentStart[2] = {centerX - radius, centerY};
    const int crescentEnd[2] = {centerX + radius, centerY};

    float xMap[width];
    const float pi_step = M_PI / (rightLimit - leftLimit);

    for (int i = leftLimit; i <= rightLimit; ++i) {
        xMap[i - leftLimit] = pi_step * (i - leftLimit);
    }

    float upperBound[rightLimit - leftLimit + 1];
    for (int i = leftLimit; i <= rightLimit; ++i) {
        upperBound[i - leftLimit] = centerY - radius * sin(xMap[i - leftLimit]) * 0.8;
    }

    float lowerBound[rightLimit - leftLimit + 1];
    for (int i = leftLimit; i <= rightLimit; ++i) {
        lowerBound[i - leftLimit] = centerY - radius * sin(xMap[i - leftLimit]);
    }

    for (int i = leftLimit; i <= rightLimit; ++i) {
        for (int j = topLimit; j <= bottomLimit; ++j) {
            if (j >= lowerBound[i - leftLimit] && j <= upperBound[i - leftLimit]) {
                image[j][i][0] = color[0];
                image[j][i][1] = color[1];
                image[j][i][2] = color[2];
            }
        }
    }

    image[crescentStart[1]][crescentStart[0]][0] = 0;
    image[crescentStart[1]][crescentStart[0]][1] = 0;
    image[crescentStart[1]][crescentStart[0]][2] = 0;

    image[crescentEnd[1]][crescentEnd[0]][0] = 0;
    image[crescentEnd[1]][crescentEnd[0]][1] = 0;
    image[crescentEnd[1]][crescentEnd[0]][2] = 0;
}

void addIris() {
    const int irisCenter[2] = {centerX, centerY};
    const int irisRadius = 10;
    const unsigned char color[3] = {255, 255, 255};

    for (int i = 0; i < IMG_HEIGHT; ++i) {
        for (int j = 0; j < IMG_WIDTH; ++j) {
            if ((i - irisCenter[1]) * (i - irisCenter[1]) + (j - irisCenter[0]) * (j - irisCenter[0]) <= irisRadius * irisRadius) {
                image[i][j][0] = color[0];
                image[i][j][1] = color[1];
                image[i][j][2] = color[2];
            }
        }
    }
}

void createWindow(void) {
    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "Conan";
    RegisterClass(&wc);

    HWND window = CreateWindow(wc.lpszClassName, "Conan", WS_OVERLAPPEDWINDOW | WS_VISIBLE, 0, 0, 800, 600, 0, 0, wc.hInstance, 0);
    CreateWindow("BUTTON", "OK", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 10, 100, 30, window, (HMENU)ID_BUTTON_OK, wc.hInstance, 0);
    CreateWindow("EDIT", "Hello, World!", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE, 10, 50, 200, 100, window, 0, wc.hInstance, 0);
    CreateWindow("BUTTON", "Quit", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 160, 100, 30, window, (HMENU)ID_BUTTON_QUIT, wc.hInstance, 0);
    CreateWindow("BUTTON", "Generate", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 200, 100, 30, window, (HMENU)ID_BUTTON_GENERATE, wc.hInstance, 0);
    CreateWindow("BUTTON", "Toggle Thread", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 240, 100, 30, window, (HMENU)ID_BUTTON_TOGGLE_BLOCK_THREAD, wc.hInstance, 0);
    CreateWindow("BUTTON", "Toggle Random Type", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 10, 280, 100, 30, window, (HMENU)ID_BUTTON_TOGGLE_GEN_TYPE, wc.hInstance, 0);

    HWND hStatic = CreateWindow("STATIC", "Generation Mode: All", WS_VISIBLE | WS_CHILD, 250, 220, 200, 30, window, 0, wc.hInstance, 0);

    srand(time(NULL));
    generateRandomImage();

    generationMode = (int*)malloc(sizeof(int));
    *generationMode = GENERATION_MODE_ALL;
    struct mode_thread_data* modeData = (struct mode_thread_data*)malloc(sizeof(struct mode_thread_data));
    modeData->generationMode = generationMode;
    modeData->hwnd = hStatic;
    thrd_t modeThread;
    thrd_create(&modeThread, manageModeLabelText, modeData);

    HDC hdc = GetDC(window);
    memDC = CreateCompatibleDC(hdc);
    bitmap = CreateCompatibleBitmap(hdc, IMG_WIDTH, IMG_HEIGHT);
    SelectObject(memDC, bitmap);

    memset(&bmi, 0, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = IMG_WIDTH;
    bmi.bmiHeader.biHeight = -IMG_HEIGHT;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    renderImage(window);
    ReleaseDC(window, hdc);

    MSG msg;
    while (1) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(1);
    }

    DeleteObject(bitmap);
    DeleteDC(memDC);
    free(generationMode);
    free(modeData);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BUTTON_OK:
                    MessageBox(hwnd, "OK Button Clicked", "Button Click", MB_OK);
                    break;
                case ID_BUTTON_QUIT:
                    PostQuitMessage(0);
                    break;
                case ID_BUTTON_GENERATE:
                    if (rand() % 100 != 0) {
                        generateRandomImage();
                    } else {
                        generateCrescentImage();
                        addIris();
                    }
                    renderImage(hwnd);
                    break;
                case ID_BUTTON_TOGGLE_BLOCK_THREAD:
                    if (keepThreadAlive == NULL) {
                        keepThreadAlive = (int*)malloc(sizeof(int));
                        *keepThreadAlive = 1;
                        struct thread_data* data = (struct thread_data*)malloc(sizeof(struct thread_data));
                        data->keepThreadAlive = keepThreadAlive;
                        data->generationMode = generationMode;
                        data->hwnd = hwnd;
                        generateRandomBlockThreaded(data);
                    } else {
                        *keepThreadAlive = 0;
                        free(keepThreadAlive);
                        keepThreadAlive = NULL;
                    }
                    break;
                case ID_BUTTON_TOGGLE_GEN_TYPE:
                    if (generationMode == NULL) {
                        generationMode = (int*)malloc(sizeof(int));
                        *generationMode = GENERATION_MODE_ALL;
                    } else {
                        *generationMode = (*generationMode == GENERATION_MODE_ALL) ? GENERATION_MODE_BATCH : GENERATION_MODE_ALL;
                    }
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

#ifdef _WIN32
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#else
int main(void)
#endif
{
    createWindow();
    return EXIT_SUCCESS;
}
