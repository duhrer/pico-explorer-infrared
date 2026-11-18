#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "pico_explorer.hpp"
#include "drivers/st7789/st7789.hpp"
#include "libraries/pico_graphics/pico_graphics.hpp"

using namespace pimoroni;

// Copied from pico-examples
#include "nec_receive_library/nec_receive.c"

# define IR_RX_GPIO_PIN 5

ST7789 st7789(PicoExplorer::WIDTH, PicoExplorer::HEIGHT, ROTATE_0, false, get_spi_pins(BG_SPI_FRONT));

// 8-bit colour
PicoGraphics_PenRGB332 graphics(st7789.width, st7789.height, nullptr);

// 16-bit colour
// PicoGraphics_PenRGB565 graphics(st7789.width, st7789.height, nullptr);

const int CX = round(st7789.width / 2);
const int CY = round(st7789.height / 2);

Pen WHITE = graphics.create_pen(255, 255, 255);
Pen BLACK = graphics.create_pen(0, 0, 0);

Pen RED = graphics.create_pen(255, 0, 0);

Pen GREEN = graphics.create_pen(0, 255, 0);
Pen LIGHTER_GREEN = graphics.create_pen(100, 255, 0);

Pen BLUE = graphics.create_pen(0,0,255);
Pen LIGHTER_BLUE = graphics.create_pen(64,64,255);
Pen LIGHTEST_BLUE = graphics.create_pen(127,127,255);

Pen ORANGE = graphics.create_pen(255, 165, 0);
Pen LIGHTER_ORANGE = graphics.create_pen(255, 213, 128);

Pen YELLOW = graphics.create_pen(255, 208, 63);

Pen BLUE_PURPLE = graphics.create_pen(200, 0, 255);
Pen RED_PURPLE = graphics.create_pen(255, 0, 255);

char waiting[4] = "...";
char error[4] = "!!!";

bool keep_running = true;

void draw_text (char textString[] ) {
    int text_width = graphics.measure_text(textString, 4, 1, true);

    int text_x = CX - round(text_width / 2);
    int text_y = CY - 16;
    Point textLocation = Point(text_x, text_y);
    Rect text_box(text_x, text_y, text_width, 32);

    graphics.set_pen(BLACK);
    graphics.rectangle(text_box);

    graphics.set_pen(WHITE);
    graphics.text(textString, textLocation, text_box.w, 4.0, 0.0, 1.0, true);
}

PIO pio = pio0;
int rx_sm;

bool isLit = true;

int colourMode = 0; // 0: Solid, 1: Discrete Rainbow 2: Continuous Rainbow

// Pico Explorer apparently doesn't have a backlight control, but maybe the new one will.
int brightness = 75;

Pen backgroundColour = BLACK;


void draw_stepped_rainbow () {
    int band_width = st7789.width / 7;
    for (int a = 0; a < 7; a++) {
        switch (a) {
            case 0:
                graphics.set_pen(RED);
                break;
            case 1:
                graphics.set_pen(ORANGE);
                break;
            case 2:
                graphics.set_pen(YELLOW);
                break;
            case 3:
                graphics.set_pen(GREEN);
                break;
            case 4:
                graphics.set_pen(BLUE);
                break;
            case 5:
                graphics.set_pen(BLUE_PURPLE);
                break;
            case 6:
                graphics.set_pen(RED_PURPLE);
                break;
        }
        Rect colour_rect((a * band_width), 0, band_width, st7789.height);
        graphics.rectangle(colour_rect);
    }
}

typedef struct {
    int r;
    int g;
    int b;
} TARGET_COLOUR;

TARGET_COLOUR TC_RED    = { .r = 255, .g = 0,   .b = 0};
TARGET_COLOUR TC_ORANGE = { .r = 255, .g = 165, .b = 0};
TARGET_COLOUR TC_YELLOW = { .r = 255, .g = 255, .b = 0};
TARGET_COLOUR TC_GREEN  = { .r = 0,   .g = 255, .b = 0};
TARGET_COLOUR TC_TEAL  =  { .r = 0,   .g = 255, .b = 255};
TARGET_COLOUR TC_BLUE   = { .r = 0,   .g = 0,   .b = 255};
TARGET_COLOUR TC_PURPLE = { .r = 255, .g = 0,   .b = 255};

TARGET_COLOUR COLOUR_WHEEL[7] = { TC_RED, TC_ORANGE, TC_YELLOW, TC_GREEN, TC_TEAL, TC_BLUE, TC_PURPLE};

void draw_continuous_rainbow () {
    int r = 255;
    int g = 0;
    int b = 0;

    for (int i = 0; i < 7; i++) {
        TARGET_COLOUR currentColour = COLOUR_WHEEL[i];

        int startX = i * 37;

        Rect colour_rect(startX, 0, startX + 19, st7789.height);       
        Pen colour_pen = graphics.create_pen(currentColour.r, currentColour.g, currentColour.b);
        graphics.set_pen(colour_pen);
        graphics.rectangle(colour_rect);

        if (i < 6) {
            TARGET_COLOUR blendColour = COLOUR_WHEEL[i + 1];
            Rect blended_rect(startX + 19, 0, startX + 37, st7789.height);
            Pen blended_pen = graphics.create_pen(
                (currentColour.r + blendColour.r)/2,
                (currentColour.g + blendColour.g)/2,
                (currentColour.b + blendColour.b)/2
            );
            graphics.set_pen(blended_pen);
            graphics.rectangle(blended_rect);
        }
    }
}

void setup () {
    st7789.set_backlight(brightness);
    graphics.set_font("bitmap8");
    graphics.set_pen(backgroundColour);
    graphics.clear();
    st7789.update(&graphics);

    stdio_init_all();

    rx_sm = nec_rx_init(pio, IR_RX_GPIO_PIN);

    if (rx_sm == -1) {
        keep_running = false;
        draw_text(error);
    }
    else {
        draw_text(waiting);
    }
}

/*

    Codes used by 'practical series ii' remote:

    00:45 - On button (at last used brightness)
    00:46 - White + Dimmed brightness
    00:47 - Off button
    00:44 - 'Stepped' colour cycling
    00:40 - Turn on timer?
    00:43 - Add one hour to timer
    00:07 - Continuous transitions between colours
    00:15 - Increase brightness
    00:09 - Decrease brightness
    00:16 - Red
    00:19 - Green
    00:0d - Blue
    00:0c - Orange
    00:18 - Light Green
    00:5e - Light Blue
    00:08 - Blueish Purple
    00:1c - Light Orange
    00:5a - Lightest Blue
    00:42 - Reddish Purple
    00:52 - Yellow
    00:4a - White + Full Brightness

    Codes used by kit's 'number pad' remote:

    00:45 - 1
    00:46 - 2
    00:47 - 3
    00:44 - 4
    00:40 - 5
    00:43 - 6
    00:07 - 7
    00:15 - 8
    00:09 - 9
    00:16 - *
    00:19 - 0
    00:0d - #
    00:0c - (not present)
    00:18 - Up Arrow
    00:5e - (not present)
    00:08 - Left Arrow
    00:1c - OK
    00:5a - Right Arrow
    00:42 - (not present)
    00:52 - Down Arrow
    00:4a - (not present)

    The LED light curtains I've used recently come with a larger controller that
    uses a different 32 bit payload. The NEC standard should be 32 bits, as in:

    uint8_t address;
    uint8_t inverted_address;
    uint8_t data;
    uint8_t inverted_data;

    The light curtain remote uses the first byte for the data, the second byte
    for its inverse, and something weird for the third and fourth byte that
    causes the NEC decode function to return `false`.  We detect this and set
    the data we care about based on the first byte of the remote's payload,
    which is:

    DA - Off
    D2 - Return to subtitle
    D9 - On
    F4 - Discrete Rainbow
    F5 - Continuous Rainbow
    F0 - Dynamic Gallery
    A4 - increase brightness
    A3 - Horizontal Flip
    91 - decrease brightness
    F9 - Increase speed
    F7 - Up Arrow
    F6 - Left Arrow
    75 - home/enter? (mountain with sun/moon)
    EF - Right Arrow
    E3 - Static Gallery Pause
    D6 - Down Arrow
    AC - Static Gallery Flicker
    F3 - Music Mode 1
    E5 - Music Mode 2
    E0 - Music Mode 3
    FE - 1 Hour Timer
    FF - 2 Hour Timer
    FD - 3 Hour Timer

    Thankfully there is no overlap in the codes used between the two.

*/

int main() {
    setup();

    uint8_t rx_address, rx_data, previous_data;
    while (keep_running) {
        // Copied from pico-examples
        while (!pio_sm_is_rx_fifo_empty(pio, rx_sm)) {
            uint32_t rx_frame = pio_sm_get(pio, rx_sm);

            bool isProperNEC = nec_decode_frame(rx_frame, &rx_address, &rx_data);

            // Workaround to enable support for second non-NEC remote.
            uint8_t data = isProperNEC ? rx_data : (rx_frame >> 24) & 0xFF;
            uint8_t address = isProperNEC ? rx_address : 0;

            if (data != previous_data) {
                int old_brightness = brightness;
                bool wasLit = isLit;

                if (isLit) {
                        switch(data) {
                        // 00:46 - White + Dimmed brightness
                        case 0x46:
                            colourMode = 0;
                            brightness = 50;
                            backgroundColour = WHITE;
                            break;
                        // 00:47 - Off button
                        case 0x47:
                        case 0xDA:
                            isLit = false;
                            break;
                        // 00:44 - 'Stepped' colour cycling
                        case 0x44:
                        case 0xF4:
                            colourMode = 1;
                            break;
                        // 00:40 - Turn on timer (ignored)
                        // 00:43 - Add one hour to timer (ignored)
                        case 0x40:
                        case 0x43:
                            break;
                        // 00:07 - Continuous transitions between colours
                        case 0x07:
                        case 0xF5:
                            colourMode = 2;
                        break;
                        // 00:15 - Increase brightness
                        case 0x15:
                        case 0xA4:
                            if (brightness <= 90) {
                                brightness += 10;
                            }
                            break;
                        // 00:09 - Decrease brightness
                        case 0x09:
                        case 0x91:
                            if (brightness >= 20) {
                                brightness -= 10;
                            }
                            break;
                        // 00:16 - Red
                        case 0x16:
                            colourMode = 0;
                            backgroundColour = RED;
                            break;
                        // 00:19 - Green
                        case 0x19:
                            colourMode = 0;
                            backgroundColour = GREEN;
                            break;
                        // 00:0d - Blue
                        case 0x0d:
                            colourMode = 0;
                            backgroundColour = BLUE;
                            break;
                        // 00:0c - Orange
                        case 0x0c:
                            colourMode = 0;
                            backgroundColour = ORANGE;
                            break;
                        // 00:18 - Light Green
                        case 0x18:
                            colourMode = 0;
                            backgroundColour = LIGHTER_GREEN;
                            break;
                        // 00:5e - Light Blue
                        case 0x5e:
                            colourMode = 0;
                            backgroundColour = LIGHTER_BLUE;
                            break;
                        // 00:08 - Blueish Purple
                        case 0x08:
                            colourMode = 0;
                            backgroundColour = BLUE_PURPLE;
                            break;
                        // 00:1c - Light Orange
                        case 0x1c:
                            colourMode = 0;
                            backgroundColour = LIGHTER_ORANGE;
                            break;
                        // 00:5a - Lightest Blue
                        case 0x5a:
                            colourMode = 0;
                            backgroundColour = LIGHTEST_BLUE;
                            break;
                        // 00:42 - Reddish Purple
                        case 0x42:
                            colourMode = 0;
                            backgroundColour = RED_PURPLE;
                            break;
                        // 00:52 - Yellow
                        case 0x52:
                            colourMode = 0;
                            backgroundColour = YELLOW;
                            break;
                        // 00:4a - White + Full Brightness
                        case 0x4a:
                            colourMode = 0;
                            backgroundColour = WHITE;
                            brightness = 100;
                            break;
                        default:
                            break;
                    }
                }
                // 00:45 - On button (at last used brightness)
                else if (data == 0x45 || data == 0xD9) {
                    isLit = true;
                }

                if (isLit) {
                    if (brightness != old_brightness) {
                        st7789.set_backlight(brightness);                        
                    }

                    if (colourMode == 0) {
                        graphics.set_pen(backgroundColour);
                        graphics.clear();
                    }
                    else if (colourMode == 1) {
                        draw_stepped_rainbow();
                    }
                    else if (colourMode == 2) {
                        draw_continuous_rainbow();
                    }

                    char addressed_data_string[5];
                    sprintf(addressed_data_string, "%02x:%02x", address, data);
                    draw_text(addressed_data_string);

                    st7789.update(&graphics);

                }
                else if (wasLit) {
                    graphics.set_pen(BLACK);
                    graphics.clear();
                    st7789.update(&graphics);
                }                    
            }

            previous_data = data;
        }

        sleep_ms(25);
    }
}