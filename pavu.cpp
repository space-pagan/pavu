/*  Author:     Zoya Samsonov
 *  Created:    December 22, 2020
 */

#include <iostream>
#include <string>
#include <cstring>
#include <curses.h>

#include "config.h"
#include "peakmonitor.h"

int main(int argc, char** argv) {
    // set up curses
    initscr();
    cbreak();
    noecho();
    timeout(20);
    curs_set(0);

    // PeakMonitor monitor = PeakMonitor(SINK_NAME, METER_RATE);
    PeakMonitor monitor = PeakMonitor(1, METER_RATE);

    while(!0) {
        // keep these scoped so that window can be resized
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
        char spaces[width+1];
        
        // if the user presses 'q' at any time, quit
        if (getch() == QUIT_CH) {
            endwin();
            exit(0);
        }
        while (monitor.q.size()) {
            // calculate the bar and draw it
            int sample = monitor.q.front();
            monitor.q.pop();

            // convert proportions of volume from x/128 to y/width
            int size = sample * ((float)width / 128.0f);
            int numspaces = width - size;

            // clear existing char buffers
            memset(bar, '\0', width+1);
            memset(spaces, '\0', width+1);

            // fill buffers as needed
            memset(bar, VOLUME_CH, size);
            memset(spaces, SILENCE_CH, numspaces);

            // print vu to screen
            if (HIDE_PERCENTAGE) {
                printf("%c%s%s%c\r", BRACKETS[0], bar, spaces, BRACKETS[1]);
            } else {
                printf("%3d%% %c%s%s%c\r", 
                        sample, BRACKETS[0], bar, spaces, BRACKETS[1]);
            }
            std::cout << std::flush;
        }
    }
}
