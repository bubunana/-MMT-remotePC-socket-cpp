#pragma once
#include "stdafx.h"

void hideOriginalButtons(HWND buttons[], HWND IPinput);
void showOriginalButtons(HWND buttons[], HWND IPinput);
void DisableCloseButton(HWND hwnd);
void EnableCloseButton(HWND hwnd);
void DisableOriginalButton(HWND buttons[]);
void EnableOriginalButton(HWND buttons[]);
void DisplayNotiBox(LPTSTR message);
void DisplayErrorBox(LPTSTR message);