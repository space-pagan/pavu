static const int METER_RATE = 50; // Hz
static const int METER_WIDTH = 100; // columns
static const bool FIT_TO_WIDTH = false; // overrides METER_WIDTH
static const bool FIT_ON_RESIZE = true; // FIT_TO_WIDTH but only if the terminal is narrowed to less than METER_WIDTH columns. Otherwise uses METER_WIDTH
static const bool HIDE_PERCENTAGE = false; // hides the vu percentage
static const bool HIDE_PREAMBLE = false; // hides connection messages
static const char QUIT_CH = 'q'; // the character which must be typed to quit
static const char VOLUME_CH = '|'; // meter portion for volume
static const char SILENCE_CH = '-'; // meter portion for silcence
static const char BRACKETS[2] = {'[', ']'}; // either side of meter
static const bool USE_COLORS = true; // use colors, if supported
static const float PERCENT_GROUPS[3] = {60, 85, 100}; // the upper percent for each color group
static const int COLOR_GROUPS[3] = {2, 3, 1}; // Green, Yellow, Red
static const bool BG_COLORS = false; // use the colors for BG, not FG
