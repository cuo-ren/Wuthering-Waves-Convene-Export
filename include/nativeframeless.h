#pragma once
#include <QAbstractNativeEventFilter>
#include <QByteArray>
#define _WINSOCKAPI_ 
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#include <dwmapi.h>

class NativeFramelessHelper : public QAbstractNativeEventFilter {
public:
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override {
        if (eventType != "windows_generic_MSG")
            return false;

        MSG* msg = static_cast<MSG*>(message);

        switch (msg->message) {
        case WM_NCCALCSIZE:
            if (msg->wParam == TRUE) {
                *result = 0;
                return true;
            }
            break;

        case WM_NCPAINT:
            *result = 0;
            return true;

        case WM_NCHITTEST: {
            const LONG borderWidth = 8; // 可调整边框宽度（你可以改）
            const LONG titleBarHeight = 40; // 标题栏高度（你可以改）
            const LONG buttonWidth = 140;   // 右上角按钮区域宽度（你可以改，通常3个按钮 3*40~45px）

            RECT winrect;
            GetWindowRect(msg->hwnd, &winrect);

            const long x = GET_X_LPARAM(msg->lParam);
            const long y = GET_Y_LPARAM(msg->lParam);

            // === 先处理四周边框 ===
            if (x >= winrect.left && x < winrect.left + borderWidth) {
                if (y >= winrect.top && y < winrect.top + borderWidth) {
                    *result = HTTOPLEFT;
                    return true;
                }
                if (y <= winrect.bottom && y > winrect.bottom - borderWidth) {
                    *result = HTBOTTOMLEFT;
                    return true;
                }
                *result = HTLEFT;
                return true;
            }
            if (x <= winrect.right && x > winrect.right - borderWidth) {
                if (y >= winrect.top && y < winrect.top + borderWidth) {
                    *result = HTTOPRIGHT;
                    return true;
                }
                if (y <= winrect.bottom && y > winrect.bottom - borderWidth) {
                    *result = HTBOTTOMRIGHT;
                    return true;
                }
                *result = HTRIGHT;
                return true;
            }
            if (y >= winrect.top && y < winrect.top + borderWidth) {
                *result = HTTOP;
                return true;
            }
            if (y <= winrect.bottom && y > winrect.bottom - borderWidth) {
                *result = HTBOTTOM;
                return true;
            }

            // === 再处理标题栏区域 ===
            if (y >= winrect.top + borderWidth && y < winrect.top + titleBarHeight) {
                // 判断是否落在右上角按钮区
                if (x > winrect.right - buttonWidth) {
                    // 在按钮区域，交给 Qt 自己处理，不做 HTCAPTION
                    break;
                }
                *result = HTCAPTION; // 系统认为这里是标题栏
                return true;
            }
            break;
        }
        }
        return false;
    }
};

void applyFakeTitleBar(HWND hWnd) {
    // 修改窗口样式：加上 WS_CAPTION + WS_SYSMENU + WS_MINIMIZEBOX
    LONG style = GetWindowLong(hWnd, GWL_STYLE);
    style |= WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    //style &= ~WS_THICKFRAME; // 可选：禁止缩放
    SetWindowLong(hWnd, GWL_STYLE, style);

    // 通知系统重新应用样式
    SetWindowPos(hWnd, nullptr, 0, 0, 0, 0,
        SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    // 使用 DWM 扩展框架，模拟标题栏高度
    MARGINS margins = { 0, GetSystemMetrics(SM_CYCAPTION), 0, 0 };
    DwmExtendFrameIntoClientArea(hWnd, &margins);
}
