#pragma once

#define MAX_HEADING_LEN 260

typedef struct
{
    wchar_t tchHeading[MAX_HEADING_LEN];
    //WCHAR tchHeading[MAX_HEADING_LEN];
    int tchLevel;

} Heading;