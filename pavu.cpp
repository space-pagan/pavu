/*  Author:     Zoya Samsonov
 *  Created:    December 22, 2020
 */

#include <iostream>
#include <string>
#include <cstring>
#include <curses.h>

#include "config.h"
#include "monitor.h"

int main(int argc, char** argv) {
    // set up curses
    initscr();
    cbreak();
    noecho();
    timeout(1000 / METER_RATE); // in ms
    curs_set(0);

    if (has_colors()) {
        start_color();
        use_default_colors();
        for (int i = 0; i < 3; i++)
            if (BG_COLORS) {
                init_pair(i+1, -1, COLOR_GROUPS[i]);
            } else {
                init_pair(i+1, COLOR_GROUPS[i], -1);
            }
    }

    Monitor monitor = 
        Monitor("bluez_sink.00_14_BE_52_54_A3.a2dp_sink", METER_RATE);

    while(!0) {
        int width = METER_WIDTH;
        if (FIT_TO_WIDTH) {
            width = HIDE_PERCENTAGE ? COLS - 2 : COLS - 7;
        } else if (FIT_ON_RESIZE) {
            if (HIDE_PERCENTAGE && COLS < METER_WIDTH + 2) {
                width = COLS - 2;
            } else if (COLS < METER_WIDTH + 7) {
                width = COLS - 7;
            }
        }
        
        char bar[width+1];
        
        // if the user presses 'q' at any time, quit
        if (getch() == QUIT_CH) {
            endwin(); // reset terminal to act as normal
            exit(0);
        }
        while (monitor.q.size()) {
            // get the current bar value
            int sample = monitor.q.front();
            monitor.q.pop();

            // convert proportions of volume from x/128 to y/width
            int size = sample * ((float)width / 128.0f);

            // print vu to screen
            if (has_colors() && USE_COLORS) {
                mvprintw(5,0,"%3d%% %c", 
                        (int)((float)sample * 100.0f / 128.0f), BRACKETS[0]);
                int start = 0;
                int len = 0;
                for (int i = 0; i < 3; i++) {
                    // calculate bar for this color group
                    if (i) start = width * PERCENT_GROUPS[i-1] / 100.0f;
                    len = std::max(std::min(
                        (int)(width * PERCENT_GROUPS[i] / 100.0f - start),
                        size - start), 0);
                    memset(bar, VOLUME_CH, len);
                    bar[len] = '\0';
                    // draw the bar in the correct color
                    attron(COLOR_PAIR(i+1));
                    printw("%s", bar);
                    attroff(COLOR_PAIR(i));
                }
                // print the silence portion without color
                memset(bar, SILENCE_CH, width - size);
                bar[width - size] = '\0';
                printw("%s%c", bar, BRACKETS[1]);
            } else {
                // generate string for entire bar at once since no color
                memset(bar, VOLUME_CH, size);
                memset(bar+size, SILENCE_CH, width-size);
                bar[width] = '\0';
                // print out accordingly
                if (HIDE_PERCENTAGE) {
                    printf("%c%s%c\r", BRACKETS[0], bar, BRACKETS[1]);
                } else {
                    printf("%3d%% %c%s%c\r", 
                            (int)((float)sample * 100.0f / 128.0f), 
                            BRACKETS[0], bar, BRACKETS[1]);
                }
                std::cout << std::flush;
            }
        }
    }
}
