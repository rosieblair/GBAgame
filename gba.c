#include <stdio.h>
#include <stdlib.h>

#include "lib.h"

#include "images/boop.h"
#include "images/spoop.h"
#include "images/melon.h"
#include "images/main_screen.h"
#include "images/background.h"
#include "images/game_over.h"



int main(void) {

	REG_DISPCNT = MODE3 | BG2_ENABLE;
	GBAState state = START; // initialize game state to start
	char count_buffer[100]; // char buffer for showing a string on screen
	static int melonCount; // number of melons boop has collected, shown on screen

    // draw a melon to collect at random place on screen
    Item melon_item;
    melon_item.x = (rand() % ITEM_ROW_RAND);
    melon_item.dx = 0;
   	melon_item.y = (rand() % ITEM_COL_RAND);
    melon_item.dy = 0;

    // draw boop at random pos on screen in range
    Item boop_item;
    boop_item.x = (rand() % ITEM_ROW_RAND);
    boop_item.dx = 1;
    boop_item.y = (rand() % ITEM_COL_RAND);
    boop_item.dy = 1;

    // draw spoop at corner pos on screen
    Item spoop_item;
    spoop_item.x = 0;
    spoop_item.dx = 1;
    spoop_item.y = 0;
    spoop_item.dy = 1;


   // LOOP - GAMEPLAY
    while (1) {
    	// wait for vblank
    	waitForVBlank();
    	// get keys for this state
    	getKey();
    	sprintf(count_buffer, "MELONS: %d", melonCount);

    	// SWITCH - STATE TRANSITIONS
    	switch(state) {
    		// STATE: WHEN PLAYER ON MAIN SCREEN/DEFAULT LAUNCH
    		case START:
    			// draw the main screen to screen
    			drawImageDMA(0, 0, MAIN_SCREEN_WIDTH, MAIN_SCREEN_HEIGHT, main_screen);
    			
    			// set to no draw state, then break
    			state = START_NODRAW; 
    			break;

    		// STATE: WHEN PLAYER ON MAIN SCREEN BEFORE PRESSING START
    		case START_NODRAW:
    			// if player presses start, change the state to game screen
    			if (isKeyDown(BUTTON_START) && wasKeyUp(BUTTON_START)) {
    				state = APP_INIT;
    			}
    			break;
    		// STATE: AFTER PRESS START ON MAIN SCREEN, INITIALIZE GAME SCREEN
    		case APP_INIT:
    			// fillScreenDMA(BLACK);
    			// draw background image to screen
    			drawFullScreenImageDMA(background);
    			// put melon count in top left corner
    			drawRectDMA(MELON_COUNT_X, MELON_COUNT_Y, CHAR_X, CHAR_Y, BLACK);
    			drawString(MELON_COUNT_X, MELON_COUNT_Y, count_buffer, WHITE);
    			// draw melon, boop, and spoop to screen
    			drawImageDMA(melon_item.x, melon_item.y, MELON_WIDTH, MELON_HEIGHT, melon);
    			drawImageDMA(boop_item.x, boop_item.y, BOOP_WIDTH, BOOP_HEIGHT, boop);
    			drawImageDMA(spoop_item.x, spoop_item.y, SPOOP_WIDTH, SPOOP_HEIGHT, spoop);

    			state = APP;
    			break;

    		// STATE: MAIN GAME SCREEN
    		case APP:
    			// check return key press -> main screen
    			if (isKeyDown(BUTTON_SELECT) && wasKeyUp(BUTTON_SELECT)) {
    				state = START;
    			}
    			// update boop and melon items pos'n on screen
    			drawImageDMA(boop_item.x, boop_item.y, BOOP_WIDTH, BOOP_HEIGHT, boop);
				drawImageDMA(melon_item.x, melon_item.y, MELON_WIDTH, MELON_HEIGHT, melon);

				// check positional boundaries of spoop - adjust if too close to edges
                if ((spoop_item.x > POS_EDGE_X && spoop_item.dx > 0) || (spoop_item.x < POS_EDGE_MIN && spoop_item.dx < 0)) {
                	spoop_item.dx = -(spoop_item.dx);
                }
                if ((spoop_item.y > POS_EDGE_Y && spoop_item.dy > 0) || (spoop_item.y < POS_EDGE_MIN && spoop_item.dy < 0)) {
                    spoop_item.dy = -(spoop_item.dy);
             	 }
		        // cover up path of spoop so he doesn't trail, then redraw spoop
		        drawRectDMA(spoop_item.x, spoop_item.y, SPOOP_HEIGHT, SPOOP_WIDTH, BLACK);
		        spoop_item.x += spoop_item.dx;
		        spoop_item.y += spoop_item.dy;
		        drawImageDMA(spoop_item.x, spoop_item.y, SPOOP_HEIGHT, SPOOP_HEIGHT, spoop);

                // d-pad directions to update boop's position
				// key press -> down arrow
				if (isKeyDown(BUTTON_DOWN) && (boop_item.x + BOOP_HEIGHT < HEIGHT)) {
                	drawRectDMA(boop_item.x, boop_item.y, BOOP_HEIGHT, BOOP_WIDTH, BLACK);
               		boop_item.x += boop_item.dx;
               	 	drawImageDMA(boop_item.x, boop_item.y, BOOP_WIDTH, BOOP_HEIGHT, boop);
                }
				// key press -> up arrow
				if (isKeyDown(BUTTON_UP) && (boop_item.x > 0)) {
                    drawRectDMA(boop_item.x, boop_item.y, BOOP_HEIGHT, BOOP_WIDTH, BLACK);
                    boop_item.x -= boop_item.dx;
                    drawImageDMA(boop_item.x, boop_item.y, BOOP_WIDTH, BOOP_HEIGHT, boop);
               	}
				// key press -> left arrow
                if (isKeyDown(BUTTON_LEFT) && (boop_item.y > 0)) {
                    drawRectDMA(boop_item.x, boop_item.y, BOOP_HEIGHT, BOOP_WIDTH, BLACK);
                    boop_item.y -= boop_item.dy;
					drawImageDMA(boop_item.x, boop_item.y, BOOP_WIDTH, BOOP_HEIGHT, boop);
                }
				// key press -> right arrow
                if (isKeyDown(BUTTON_RIGHT) && (boop_item.y + BOOP_WIDTH < WIDTH)) {
                    drawRectDMA(boop_item.x, boop_item.y, BOOP_HEIGHT, BOOP_WIDTH, BLACK);
                    boop_item.y += boop_item.dy;
					drawImageDMA(boop_item.x, boop_item.y, BOOP_WIDTH, BOOP_HEIGHT, boop);
                }

				// check if 2 ghosts have interacted, game over
				if (wasCollision(boop_item.x, boop_item.y, BOOP_HEIGHT, BOOP_WIDTH,
						spoop_item.x, spoop_item.y, SPOOP_HEIGHT, SPOOP_WIDTH)) {
					state = APP_EXIT;
				}
				
				// check if boop has collected a melon via touch, update display count if yes
				if (wasCollision(boop_item.x, boop_item.y, BOOP_HEIGHT, BOOP_WIDTH,
						melon_item.x, melon_item.y, MELON_HEIGHT, MELON_WIDTH)) {
					// cover old position to move to new and old count
					drawRectDMA(melon_item.x, melon_item.y, MELON_HEIGHT, MELON_WIDTH, BLACK);
					melonCount++; // increment melon count
					// calculate new random position for melon
					melon_item.x = (rand() % MELON_ROW_RAND);
					melon_item.y = (rand() % MELON_COL_RAND);
					// draw updated items
					drawRectDMA(MELON_COUNT_X, MELON_COUNT_Y, CHAR_X, CHAR_Y, BLACK);
					drawImageDMA(melon_item.x, melon_item.y, MELON_WIDTH, MELON_HEIGHT, melon);
					sprintf(count_buffer, "MELONS: %d", melonCount);
    				drawString(MELON_COUNT_X, MELON_COUNT_Y, count_buffer, WHITE);			
				}
				
				break;
				
			// STATE: INIT GAME OVER SCREEN:
			case APP_EXIT:
				drawImageDMA(0, 0, GAME_OVER_WIDTH, GAME_OVER_HEIGHT, game_over);
				state = APP_EXIT_NODRAW;
				break;
				
			// STATE: GAME OVER SCREEN - STAY:
			case APP_EXIT_NODRAW:
				if (isKeyDown(BUTTON_START) && wasKeyUp(BUTTON_START)) {
					state = APP_TRY_AGAIN;
				}
				break;
			
			// STATE: PLAYER RESTARTED FROM END SCREEN:
			case APP_TRY_AGAIN:
				melonCount = 0;
				melon_item.x = (rand() % ITEM_ROW_RAND);
				melon_item.y = (rand() % ITEM_COL_RAND);
				boop_item.x = (rand() % ITEM_ROW_RAND);
				boop_item.y = (rand() % ITEM_COL_RAND);
				spoop_item.x = (rand() % ITEM_ROW_RAND);
				spoop_item.y = (rand() % ITEM_COL_RAND);
				state = APP_INIT;
				
				break;
    	}

    }

}
