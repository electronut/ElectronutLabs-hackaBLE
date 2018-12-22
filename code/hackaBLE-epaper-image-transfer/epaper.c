/** @file epaper.c
 *
 * @brief E-paper functions abstraction
 *
 * @par
 * COPYRIGHT NOTICE: (c) 2018 Electronut Labs.
 * All rights reserved. 
*/
 
#include "epaper.h"

#include "epd1in54b.h"
#include "epdif.h"
#include "epdpaint.h"
#include "image.h"
#include "fonts.h"

EPD epd;

unsigned char frame_buffer_black_arr[EPD_WIDTH * EPD_HEIGHT / 8];
unsigned char *frame_buffer_black = frame_buffer_black_arr;
unsigned char frame_buffer_red_arr[EPD_WIDTH * EPD_HEIGHT / 8];
unsigned char *frame_buffer_red = frame_buffer_red_arr;

Paint paint_black;
Paint paint_red;

int epaper_init()
{
    epaper_GPIO_Init();
    spi_epd_init();
    
    return EPD_Init(&epd);
}

void epaper_test()
{
    Paint_Init(&paint_black, frame_buffer_black, epd.width, epd.height);
    Paint_Init(&paint_red, frame_buffer_red, epd.width, epd.height);
    Paint_Clear(&paint_black, UNCOLORED);
    Paint_Clear(&paint_red, UNCOLORED);

    /* Display the frame_buffer */
    Paint_SetRotate(&paint_black, 3);
    Paint_DrawStringAt(&paint_black, 20, 90, "EPAPER TEST", &Font20, COLORED);
    
    EPD_DisplayFrame(&epd, frame_buffer_black, frame_buffer_red);
}

void epaper_clear()
{
    Paint_Init(&paint_black, frame_buffer_black, epd.width, epd.height);
    Paint_Init(&paint_red, frame_buffer_red, epd.width, epd.height);
    Paint_Clear(&paint_black, UNCOLORED);
    Paint_Clear(&paint_red, UNCOLORED);

    EPD_DisplayFrame(&epd, frame_buffer_black, frame_buffer_red);
}