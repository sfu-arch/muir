#ifndef _ROI_H
#define  _ROI_H

#pragma once 
#include <stdio.h>
#include <stdint.h>

/* General Markers */

void __attribute__ ((noinline)) __app_roi_begin()
{
    asm("");
}

void __attribute__ ((noinline)) __app_roi_end()
{
    asm("");
}

#endif
