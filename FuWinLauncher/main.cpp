#include <windows.h>
#include "App.h"

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR, int) {
    App app;
    if (!app.Init(hInstance)) {
        return 1;
    }
    return app.Run();
}
