/** @file epaper.h
 *
 * @brief E-paper functions abstraction
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2018 Electronut Labs.
 * All rights reserved. 
*/
 
#ifndef _ADM_EPAPER_H
#define _ADM_EPAPER_H
 
#ifdef __cplusplus 
extern "C" { 
#endif

#include "epd1in54b.h"
#include "epdpaint.h"

extern EPD epd;
extern unsigned char *frame_buffer_black;
extern unsigned char *frame_buffer_red;
extern Paint paint_black;
extern Paint paint_red;

/**
 * @brief Initialize epaper display
 * 
 * @return int 
 */
int epaper_init();

/**
 * @brief clear epaper display
 * 
 */
void epaper_clear();

/**
 * @brief display default image
 * 
 */
void epaper_default_image();

#ifdef __cplusplus 
}
#endif

#endif