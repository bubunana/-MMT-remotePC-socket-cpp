#pragma once
#include "Heading.h"
#include "stdafx.h"

struct Process {
    WCHAR name[MAX_HEADING_LEN];
    DWORD id;
    DWORD threadCount;
};
