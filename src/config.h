#ifndef PROCASM_CONFIG_H
#define PROCASM_CONFIG_H

// Milliseconds per clock cycle.
constexpr int TICK_DELAY = 2;

#define TEXT_COLOR 0xf0, 0xf0, 0xf0, 0xff

constexpr int WIDTH = 1920, HEIGHT = 1080;

constexpr int MAX_LINE_WIDTH = 60;
constexpr int MAX_LINES = 40;


constexpr int BOX_LINE_HEIGHT = 20;
constexpr int BOX_TEXT_MARGIN = 16;
constexpr int BOX_CHAR_WIDTH = 8;
constexpr int BOX_W = 2 * BOX_TEXT_MARGIN + MAX_LINE_WIDTH * BOX_CHAR_WIDTH;
constexpr int BOX_H = 2* BOX_TEXT_MARGIN + MAX_LINES * BOX_LINE_HEIGHT;
constexpr int BOX_X = WIDTH - BOX_W - 50;
constexpr int BOX_Y = HEIGHT / 2 - BOX_H / 2;
constexpr int BOX_UNDO_BUFFER_SIZE = 1024;

constexpr int SPACES_PER_TAB = 4;

#endif // PROCASM_CONFIG_H
