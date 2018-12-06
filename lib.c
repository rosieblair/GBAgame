#include "lib.h"

volatile unsigned short *videoBuffer = (volatile unsigned short *)0x6000000;
u32 vBlankCounter = 0;

// button key stuff
u16 curr_key = 0;
u16 prev_key = 0;

// WAIT FOR VBLANK: avoid screen tearing & maintain motion fluidity
void waitForVBlank(void) {
    while (SCANLINECOUNTER > 160);
    while (SCANLINECOUNTER < 160);
}

static int __qran_seed= 42;
static int qran(void) {
    __qran_seed= 1664525*__qran_seed+1013904223;
    return (__qran_seed>>16) & 0x7FFF;
}

int randint(int min, int max) {
    return (qran()*(max-min)>>15)+min;
}

// Get the key button and set previous key to current
void getKey(void) {
    prev_key = curr_key;
    curr_key = ~BUTTONS & KEY_MASK;

}

// Checks if given key is currently down
u32 isKeyDown(u32 key) {
    return curr_key & key;
}

// Checks if given key is currently up
u32 isKeyUp(u32 key) {
    return ~curr_key & key;
}

// Checks if given key was previously down
u32 wasKeyDown(u32 key) {
    return prev_key & key;
}

// Checks if given key was previously up
u32 wasKeyUp(u32 key) {
    return ~prev_key & key;
}

// Sets a pixel to the color on the screen.
// INPUTS: coordinates (x, y), the color
void setPixel(int x, int y, u16 color) {
    videoBuffer[OFFSET(y, x, WIDTH)] = color;
}

// Draws a filled rectangle of color to the screen.
// INPUTS: coordinates (x, y), rectangle's dimensions, color
void drawRectDMA(int x, int y, int width, int height, volatile u16 color) {
    for (int r = 0; r < height; r++) {
        DMA[3].src = &color;
        DMA[3].dst = videoBuffer + OFFSET(x+r, y, 240);
        DMA[3].cnt = (width) | DMA_ON | DMA_DESTINATION_INCREMENT | DMA_SOURCE_FIXED;
    }
}

// Fills the entire screen with an image (sets it as background).
// INPUTS: the image
void drawFullScreenImageDMA(const u16 *image) {
    DMA[3].src = image;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = (AREA) | DMA_ON | DMA_SOURCE_INCREMENT | DMA_DESTINATION_INCREMENT;
}

// Draws an image to the screen.
// INPUTS: coordinates (x, y), image's dimensions, the image
void drawImageDMA(int x, int y, int width, int height, const u16 *image) {
    for (int i = 0; i < height; i++) {
    	DMA[3].src = image + OFFSET(i, 0, width);
    	DMA[3].dst = videoBuffer + OFFSET((i+x), y, 240);
    	DMA[3].cnt = width | DMA_ON;
    }
}

// Fills the entire screen with a given color.
// INPUTS: the color
void fillScreenDMA(volatile u16 color) {
    DMA[3].src = &color;
    DMA[3].dst = videoBuffer;
    DMA[3].cnt = (AREA) | DMA_ON | DMA_SOURCE_FIXED;
}

// Checks for collision at the given item coordinates
// If there is an overlap/interaction, then register as collision (return 1)
// Else, no collision (return 0)
int wasCollision(int x1, int y1, int dx1, int dy1, int x2, int y2, int dx2, int dy2) {
    // checks if the items at the coordinates touch/overlap -> collision if yes
    if ((x1 < (x2 + dx2)) && ((x1 + dx1) > x2) && (y1 < (y2 + dy2)) && ((y1 + dy1) > y2)) {
        return 1;
    } else {
        return 0;
    }
}

void drawChar(int col, int row, char ch, u16 color) {
    for(int r = 0; r<8; r++) {
        for(int c=0; c<6; c++) {
            if(fontdata_6x8[OFFSET(r, c, 6) + ch*48]) {
                setPixel(col+c, row+r, color);
            }
        }
    }
}

void drawString(int col, int row, char *str, u16 color) {
    while(*str) {
        drawChar(col, row, *str++, color);
        col += 6;
    }
}

void drawCenteredString(int x, int y, int width, int height, char *str, u16 color) {
    u32 len = 0;
    char *strCpy = str;
    while (*strCpy) {
        len++;
        strCpy++;
    }

    u32 strWidth = 6 * len;
    u32 strHeight = 8;

    int col = x + ((width - strWidth) >> 1);
    int row = y + ((height - strHeight) >> 1);
    drawString(col, row, str, color);
}

