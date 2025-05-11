// lab-01-galton-board.c - Galton Board digital com múltiplas bolas e curva de Gauss
// O Display OLED está conectado ao barramento I2C da BitDogLab através dos seguintes pinos:
// SDA: GPIO14
// SCL: GPIO15
// O endereço do Display OLED é 0x3C.
// Passo 3: Escrevendo o Código

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "include/ssd1306.h"

#define I2C_PORT i2c0
#define I2C_SDA 14
#define I2C_SCL 15
#define OLED_ADDR 0x3C
#define BTN_CURVE 5
#define BTN_RESET 6

#define LARGURA_DISPLAY 128
#define ALTURA_DISPLAY 64
#define MAX_BOLAS 20
#define NIVEIS 10
#define COLUNAS (NIVEIS + 1)
#define ESPACAMENTO (LARGURA_DISPLAY / COLUNAS)
#define ALTURA_HISTOGRAMA 40

// Estrutura de uma bola
typedef struct {
    float x, y;
    int nivel;
    bool ativa;
} Bola;

Bola bolas[MAX_BOLAS];
int caixas[COLUNAS];
int total_bolas = 0;
bool mostrar_curva = false;
char texto[32];

void inicializar_display() {
    printf("Inicializando display...");
    
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init_bm(&display, 128, 64, false, OLED_ADDR, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_clear();
    ssd1306_show();
    printf("Display pronto e limpo!");
}

void inicializar_botoes() {
    gpio_init(BTN_CURVE);
    gpio_set_dir(BTN_CURVE, GPIO_IN);
    gpio_pull_down(BTN_CURVE);

    gpio_init(BTN_RESET);
    gpio_set_dir(BTN_RESET, GPIO_IN);
    gpio_pull_down(BTN_RESET);
}

void resetar_simulacao() {
    memset(caixas, 0, sizeof(caixas));
    memset(bolas, 0, sizeof(bolas));
    total_bolas = 0;
    mostrar_curva = false;
}

int escolha_binaria(float peso_direita) {
    return ((float)rand() / RAND_MAX) < peso_direita ? 1 : 0;
}

void adicionar_bola() {
    for (int i = 0; i < MAX_BOLAS; i++) {
        if (!bolas[i].ativa) {
            bolas[i].x = COLUNAS / 2.0;
            bolas[i].y = 0;
            bolas[i].nivel = 0;
            bolas[i].ativa = true;
            total_bolas++;
            break;
        }
    }
}

void atualizar_bolas() {
    for (int i = 0; i < MAX_BOLAS; i++) {
        if (bolas[i].ativa) {
            if (bolas[i].nivel < NIVEIS) {
                bolas[i].x += escolha_binaria(0.5);
                bolas[i].nivel++;
            } else {
                caixas[(int)(bolas[i].x)]++;
                bolas[i].ativa = false;
            }
        }
    }
}

void desenhar_histograma() {
    int max = 1;
    for (int i = 0; i < COLUNAS; i++) {
        if (caixas[i] > max) max = caixas[i];
    }

    for (int i = 0; i < COLUNAS; i++) {
        int altura = (caixas[i] * ALTURA_HISTOGRAMA) / max;
        for (int y = 0; y < altura; y++) {
            ssd1306_draw_pixel(i * ESPACAMENTO + 2, ALTURA_DISPLAY - 1 - y);
        }

        if (mostrar_curva) {
            float media = NIVEIS / 2.0;
            float desvio = sqrt(NIVEIS) / 2.0;
            float expoente = -pow(i - media, 2) / (2 * desvio * desvio);
            int curva_y = (int)(exp(expoente) * ALTURA_HISTOGRAMA);
            ssd1306_draw_pixel(i * ESPACAMENTO + 2, ALTURA_DISPLAY - 1 - curva_y);
        }
    }
}

#include "include/ssd1306_i2c.h"

ssd1306_t display;

void desenhar_interface() {
    snprintf(texto, sizeof(texto), "Bolas: %d", total_bolas);
    ssd1306_draw_string(display.ram_buffer + 1, 0, 0, texto);
}

int main() {
    printf("Iniciando Galton Board...");
    stdio_init_all();
    inicializar_display();
    sleep_ms(100);
    inicializar_botoes();
    resetar_simulacao();
    srand(time_us_32());

    while (true) {
        if (gpio_get(BTN_RESET)) {
            resetar_simulacao();
            sleep_ms(200);
        }
        if (gpio_get(BTN_CURVE)) {
            mostrar_curva = true;
            sleep_ms(200);
        }

        adicionar_bola();
        atualizar_bolas();

        ssd1306_clear();
        desenhar_histograma();
        desenhar_interface();
        ssd1306_show();
        sleep_ms(150);
    }
    return 0;
}
