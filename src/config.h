#ifndef PROCASM_CONFIG_H
#define PROCASM_CONFIG_H

// Milliseconds per clock cycle.
constexpr int TICK_DELAY = 2;

#define TEXT_COLOR 0xf0, 0xf0, 0xf0, 0xff

constexpr int WIDTH = 1920, HEIGHT = 1080;

constexpr int MAX_LINE_WIDTH = 24;
constexpr int MAX_LINES = 16;


constexpr int BOX_SIZE = 348;
constexpr int BOX_X = WIDTH / 2 - BOX_SIZE / 2 + 100;
constexpr int BOX_Y = HEIGHT / 2 - BOX_SIZE / 2;
constexpr int BOX_TEXT_MARGIN = 16;
constexpr int BOX_LINE_HEIGHT = 20;
constexpr int BOX_CHAR_WIDTH = 8;
constexpr int BOX_UNDO_BUFFER_SIZE = 1024;

constexpr int SPACES_PER_TAB = 4;

#endif // PROCASM_CONFIG_H
