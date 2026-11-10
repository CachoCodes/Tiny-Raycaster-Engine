#include <windows.h>
#include <iostream>
#include <chrono>
int frames = 0;
float fps = 0.0f;
auto last_time = std::chrono::high_resolution_clock::now();
float delta = 0.f;
constexpr float PI = 3.14159f;

#include "renderer.hpp"




BITMAPINFO bitmap_info{};



LRESULT CALLBACK window_proc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_SIZE:
            return 0;

        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_KEYDOWN:
            if (wParam == 'E')
                debug_mode = !debug_mode;
            if(wParam == 'D')
                raycaster::d_pressed = true;
            if(wParam == 'A')
                raycaster::a_pressed = true;
            if(wParam == 'S')
                raycaster::s_pressed = true;
            if(wParam == 'W')
                raycaster::w_pressed = true;
            return 0;
        case WM_KEYUP:
            if(wParam == 'D')
                raycaster::d_pressed = false;
            if(wParam == 'A')
                raycaster::a_pressed = false;
            if(wParam == 'S')
                raycaster::s_pressed = false;
            if(wParam == 'W')
                raycaster::w_pressed = false;

            return 0;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);

    }
}


int WINAPI WinMain(HINSTANCE h_instance, HINSTANCE, LPSTR, int) {
    WNDCLASSA window_class{};

    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = h_instance;
    window_class.lpszClassName = "Tiny-Raycaster-Engine";
    window_class.hCursor = LoadCursor(nullptr, IDC_ARROW);

    if (!RegisterClassA(&window_class)) {
        return 0;
    }


    HWND hwnd = CreateWindowExA(
        0,
        "Tiny-Raycaster-Engine",
        "Tiny-Raycaster-Engine",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        win_width,
        win_height,
        nullptr,
        nullptr,
        h_instance,
        nullptr
    );

    if (!hwnd) {

        return 0;
    }


    bitmap_info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bitmap_info.bmiHeader.biPlanes = 1;
    bitmap_info.bmiHeader.biBitCount = 32;
    bitmap_info.bmiHeader.biCompression = BI_RGB;



    ShowWindow(hwnd, SW_SHOW);


    bool running = true;

    init_render();

    if(debug_mode){
        AllocConsole();
        FILE* fp;
        freopen_s(&fp, "CONOUT$", "w", stdout);
        freopen_s(&fp, "CONOUT$", "w", stderr);
        freopen_s(&fp, "CONIN$", "r", stdin);
    }

    auto last_frame_time = std::chrono::high_resolution_clock::now();
    auto fps_time = last_frame_time;

    while (running) {
        MSG msg{};

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                running = false;
                break;
            }

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (!running) {
            break;
        }

        auto frame_start = std::chrono::high_resolution_clock::now();

        delta = std::chrono::duration<float>(frame_start - last_frame_time).count();

        last_frame_time = frame_start;
        input_loop();
        render_loop(framebuffer);

        frames++;

        auto now = std::chrono::high_resolution_clock::now();

    float elapsed = std::chrono::duration<float>(
        now - fps_time
    ).count();

    if (elapsed >= 1.0f) {
        fps = frames / elapsed;
        frames = 0;
        fps_time = now;

        char title[64];
        std::snprintf(title, sizeof(title),
            "Tiny-Raycaster-Engine - %.0f FPS", fps);

        SetWindowTextA(hwnd, title);
    }

    HDC hdc = GetDC(hwnd);

    bitmap_info.bmiHeader.biWidth = win_width;
    bitmap_info.bmiHeader.biHeight = win_height;

    StretchDIBits(
        hdc,
        0, 0,
        win_width, win_height,
        0, 0,
        win_width, win_height,
        framebuffer,
        &bitmap_info,
        DIB_RGB_COLORS,
        SRCCOPY
    );

    ReleaseDC(hwnd, hdc);
    }


    delete[] framebuffer;
    framebuffer = nullptr;

    return 0;
}