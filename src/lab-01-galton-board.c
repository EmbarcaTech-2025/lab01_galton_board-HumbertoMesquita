#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "../include/ssd1306_i2c.h"

#define I2C_SDA 14
#define I2C_SCL 15

#define BOARD_WIDTH 128
#define BOARD_HEIGHT 64
#define PIN_ROWS 7
#define PIN_SPACING_X 8
#define PIN_SPACING_Y 6
#define PIN_RADIUS 1
#define BALL_RADIUS 1
#define MAX_BALLS 20
#define HISTOGRAM_BINS 16
#define BIN_WIDTH (BOARD_WIDTH / HISTOGRAM_BINS)
#define HISTOGRAM_HEIGHT 20
#define HISTOGRAM_BASE (BOARD_HEIGHT - 1)
#define BALL_SPEED 1
#define NEW_BALL_DELAY 30

typedef struct {
    float x, y;
    int active;
    int current_row;
} Ball;

Ball balls[MAX_BALLS];
uint32_t histogram[HISTOGRAM_BINS] = {0};
uint32_t total_balls = 0;
uint32_t tick_counter = 0;

int random_binary() {
    return rand() % 2;
}

void init_ball(Ball *ball) {
    *ball = (Ball){ .x = BOARD_WIDTH / 2, .y = 0, .active = 1, .current_row = 0 };
}

void update_ball(Ball *ball) {
    if (!ball->active) return;

    ball->y += BALL_SPEED;
    int row_y = (ball->current_row + 1) * PIN_SPACING_Y;

    if (ball->y >= row_y && ball->current_row < PIN_ROWS) {
        ball->x += random_binary() ? PIN_SPACING_X / 2 : -PIN_SPACING_X / 2;
        ball->current_row++;
    }

    if (ball->y >= BOARD_HEIGHT - HISTOGRAM_HEIGHT - BALL_RADIUS) {
        int bin = (int)ball->x / BIN_WIDTH;
        if (bin < 0) bin = 0;
        if (bin >= HISTOGRAM_BINS) bin = HISTOGRAM_BINS - 1;
        histogram[bin]++;
        total_balls++;
        ball->active = 0;
    }
}

void draw_circle(uint8_t *buffer, int x, int y, int radius) {
    for (int dy = -radius; dy <= radius; dy++) {
        for (int dx = -radius; dx <= radius; dx++) {
            int px = x + dx, py = y + dy;
            if (px >= 0 && px < BOARD_WIDTH && py >= 0 && py < BOARD_HEIGHT) {
                ssd1306_set_pixel(buffer, px, py, true);
            }
        }
    }
}

void draw_histogram(uint8_t *buffer) {
    uint32_t max_count = 1;
    for (int i = 0; i < HISTOGRAM_BINS; i++) {
        if (histogram[i] > max_count) max_count = histogram[i];
    }

    for (int i = 0; i < HISTOGRAM_BINS; i++) {
        int height = (histogram[i] * HISTOGRAM_HEIGHT) / max_count;
        int x_start = i * BIN_WIDTH;
        int x_end = x_start + BIN_WIDTH - 1;

        for (int x = x_start; x <= x_end; x++) {
            for (int y = HISTOGRAM_BASE; y > HISTOGRAM_BASE - height; y--) {
                ssd1306_set_pixel(buffer, x, y, true);
            }
        }
    }

    for (int x = 0; x < BOARD_WIDTH; x++) {
        ssd1306_set_pixel(buffer, x, HISTOGRAM_BASE - HISTOGRAM_HEIGHT, true);
    }
}

void draw_pins(uint8_t *buffer) {
    for (int row = 0; row < PIN_ROWS; row++) {
        int y = (row + 1) * PIN_SPACING_Y;
        int pins = row + 1;
        int start_x = (BOARD_WIDTH - (pins - 1) * PIN_SPACING_X) / 2;

        for (int i = 0; i < pins; i++) {
            draw_circle(buffer, start_x + i * PIN_SPACING_X, y, PIN_RADIUS);
        }
    }
}

void draw_ball_count(uint8_t *buffer) {
    char str[20];
    sprintf(str, "%lu", total_balls);
    ssd1306_draw_string(buffer, 0, 0, str);
}

void update_display(uint8_t *buffer) {
    memset(buffer, 0, ssd1306_width * ssd1306_n_pages);

    struct render_area area = {
        .start_column = 0,
        .end_column = ssd1306_width - 1,
        .start_page = 0,
        .end_page = ssd1306_n_pages - 1
    };

    draw_pins(buffer);

    for (int i = 0; i < MAX_BALLS; i++) {
        if (balls[i].active) {
            draw_circle(buffer, (int)balls[i].x, (int)balls[i].y, BALL_RADIUS);
        }
    }

    draw_histogram(buffer);
    draw_ball_count(buffer);

    calculate_render_area_buffer_length(&area);
    render_on_display(buffer, &area);
}

int main() {
    stdio_init_all();
    srand(time(NULL));

    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init();

    uint8_t buffer[ssd1306_width * ssd1306_n_pages] = {0};
    memset(balls, 0, sizeof(balls));

    while (true) {
        tick_counter++;

        if (tick_counter % NEW_BALL_DELAY == 0) {
            for (int i = 0; i < MAX_BALLS; i++) {
                if (!balls[i].active) {
                    init_ball(&balls[i]);
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_BALLS; i++) {
            update_ball(&balls[i]);
        }

        update_display(buffer);
        sleep_ms(50);
    }

    return 0;
}
