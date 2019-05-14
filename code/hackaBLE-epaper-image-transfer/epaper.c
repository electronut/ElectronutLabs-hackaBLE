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

// Display default image
void epaper_default_image()
{
    Paint_Init(&paint_black, frame_buffer_black, epd.width, epd.height);
    Paint_Init(&paint_red, frame_buffer_red, epd.width, epd.height);
    Paint_Clear(&paint_black, UNCOLORED);
    Paint_Clear(&paint_red, UNCOLORED);
    
	Paint_SetRotate(&paint_red, 3);
    Paint_DrawStringAt(&paint_red, 40, 40, "hackBLE", &Font24, COLORED);
    Paint_DrawBitmap(&paint_black, 100, 0, bwData, image_width, image_height, COLORED);

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