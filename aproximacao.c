#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "hardware/adc.h"
#include "pico/bootrom.h"
#include "hardware/pwm.h"

// Definição dos LEDs RGB
#define R_LED 13
#define G_LED 11
#define B_LED 12

// Definição dos botões
#define A_BUTTON 5
#define B_BUTTON 6

#define A_BUZZER 21
#define B_BUZZER 10

static volatile bool deactivate = 0;

static volatile uint32_t last_time = 0; 

void init_all() {
    gpio_init(R_LED);
    gpio_set_dir(R_LED, GPIO_OUT);
    gpio_put(R_LED, 0);

    gpio_init(G_LED);
    gpio_set_dir(G_LED, GPIO_OUT);
    gpio_put(G_LED, 0);

    gpio_init(B_LED);
    gpio_set_dir(B_LED, GPIO_OUT);
    gpio_put(B_LED, 0);

    gpio_init(A_BUTTON);
    gpio_set_dir(A_BUTTON, GPIO_IN);
    gpio_pull_up(A_BUTTON);

    gpio_init(B_BUTTON);
    gpio_set_dir(B_BUTTON, GPIO_IN);
    gpio_pull_up(B_BUTTON);

    gpio_init(A_BUZZER);
    gpio_set_dir(A_BUZZER, GPIO_OUT);

    gpio_init(B_BUZZER);
    gpio_set_dir(B_BUZZER, GPIO_OUT);

}

void get_led(bool R, bool G, bool B) {
    gpio_put(R_LED, R);
    gpio_put(G_LED, G);
    gpio_put(B_LED, B);
}

// Inicializa o sistema de clock
void inicializar_clock() {
    bool ok = set_sys_clock_khz(100000, false);
    if (ok) {
        printf("Clock set to %ld\n", clock_get_hz(clk_sys));
    } else {
        printf("Falha ao configurar o clock\n");
    }
}

// Função de callback para desligar o LED após o tempo programado.
int64_t turn_off_callback(alarm_id_t id, void *user_data) {

    printf("Alarme ativado. \n");

    if (gpio_get(B_LED)){
        get_led(0,0,0);
    }
    else{
        gpio_put(R_LED, 1);
        gpio_put(A_BUZZER, 1);
        gpio_put(B_BUZZER, 1);
    }

}

void gpio_irq_handler(uint gpio, uint32_t events){

    uint32_t current_time = to_us_since_boot(get_absolute_time());
        if (current_time - last_time > 200000){
            last_time = current_time;

            if (gpio == B_BUTTON){

                gpio_put(B_LED, !gpio_get(B_LED));
                gpio_put(A_BUZZER, 0);
                gpio_put(B_BUZZER, 0);
               
            }

        }
}

// Função principal
int main() {
    // Inicializa clock, stdio e configurações
    stdio_init_all();
    init_all();
    inicializar_clock();

    printf("Sistema inicializado.\n");

    // Configuração dos botões como interrupções
    //gpio_set_irq_enabled_with_callback(A_BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(B_BUTTON, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    while (true) {

        if (!gpio_get(A_BUTTON)){

            sleep_ms(50);

            printf("Alarme começando.\n");

            gpio_put(R_LED, 0);

            add_alarm_in_ms(1000, turn_off_callback, NULL, false);

        }

        sleep_ms(50);

    }

    return 0;
}