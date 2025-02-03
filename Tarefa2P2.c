#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

// Definir os pinos dos LEDs e do botão
#define LED_VERMELHO_PIN  12  // GPIO 12 (vermelho)
#define LED_AZUL_PIN      13  // GPIO 13 (azul)
#define LED_VERDE_PIN     11  // GPIO 11 (verde)
#define BOTAO_PIN         5   // GPIO 5 (botão)

// Estados da sequência de LEDs
#define TODOS_LIGADOS      0
#define VERDE_DESLIGADO    1
#define VERMELHO_DESLIGADO 2
#define AZUL_DESLIGADO     3
#define TODOS_DESLIGADOS   4

int estado_atual = TODOS_DESLIGADOS; // Começa com todos os LEDs desligados
bool temporizador_ativo = false;     // Controla se o temporizador está ativo

// Função de callback para o alarme
int64_t callback_alarme(alarm_id_t id, void *user_data) {
    switch (estado_atual) {
        case TODOS_LIGADOS:
            gpio_put(LED_AZUL_PIN, 0); // Desliga o azul primeiro
            estado_atual = AZUL_DESLIGADO;
            printf("Mudança de estado: TODOS_LIGADOS -> AZUL_DESLIGADO\n");
            break;
        case AZUL_DESLIGADO:
            gpio_put(LED_VERMELHO_PIN, 0); // Desliga o vermelho
            estado_atual = VERMELHO_DESLIGADO;
            printf("Mudança de estado: AZUL_DESLIGADO -> VERMELHO_DESLIGADO\n");
            break;
        case VERMELHO_DESLIGADO:
            gpio_put(LED_VERDE_PIN, 0); // Desliga o verde por último
            estado_atual = TODOS_DESLIGADOS;
            temporizador_ativo = false; // Finaliza o ciclo
            printf("Mudança de estado: VERMELHO_DESLIGADO -> TODOS_DESLIGADOS\n");
            return 0; // Cancela o alarme
    }

    // Reagendar o alarme para 3 segundos depois
    add_alarm_in_ms(3000, callback_alarme, NULL, false);
    return 0;
}

// Função de debounce para o botão
bool debounce() {
    static uint32_t ultimo_tempo = 0;
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    if (tempo_atual - ultimo_tempo < 200) { // 200ms para debounce
        return false;
    }

    ultimo_tempo = tempo_atual;
    return true;
}

// Função principal
int main() {
    stdio_init_all(); // Inicializa a comunicação serial

    // Inicializar os pinos dos LEDs como saída
    gpio_init(LED_VERMELHO_PIN);
    gpio_init(LED_AZUL_PIN);
    gpio_init(LED_VERDE_PIN);
    gpio_set_dir(LED_VERMELHO_PIN, GPIO_OUT);
    gpio_set_dir(LED_AZUL_PIN, GPIO_OUT);
    gpio_set_dir(LED_VERDE_PIN, GPIO_OUT);

    // Inicializar o pino do botão como entrada
    gpio_init(BOTAO_PIN);
    gpio_set_dir(BOTAO_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_PIN); // Configuração de pull-up

    // Inicialmente, todos os LEDs desligados
    gpio_put(LED_VERMELHO_PIN, 0);
    gpio_put(LED_AZUL_PIN, 0);
    gpio_put(LED_VERDE_PIN, 0);

    printf("Estado inicial: TODOS_DESLIGADOS\n");

    // Loop principal
    while (true) {
        // O botão está em pull-up, então a condição de clique é quando ele está em nível baixo (0)
        if (!gpio_get(BOTAO_PIN) && debounce() && estado_atual == TODOS_DESLIGADOS) {
            temporizador_ativo = true;

            // Liga todos os LEDs
            gpio_put(LED_VERMELHO_PIN, 1);
            gpio_put(LED_AZUL_PIN, 1);
            gpio_put(LED_VERDE_PIN, 1);

            // Define o estado inicial da sequência
            estado_atual = TODOS_LIGADOS;

            // Inicia o temporizador para mudar o estado a cada 3 segundos
            add_alarm_in_ms(3000, callback_alarme, NULL, false);
        }

        sleep_ms(100); // Pequeno atraso para evitar leitura excessiva do botão
    }

    return 0;
}
