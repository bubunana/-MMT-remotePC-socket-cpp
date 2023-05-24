#pragma once
#include "framework.h"
#include <vector>


using namespace std;

class LiveScreen {
public:
    vector<BYTE> data;
public:
    bool gdiscreen();
};