#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/time.h"

// Definir os pinos dos LEDs e do botão
#define LED_VERMELHO_PIN  12  // GPIO 12 (vermelho)
#define LED_VERDE_PIN     11  // GPIO 11 (verde)
#define LED_AZUL_PIN      13  // GPIO 13 (azul)
#define BOTAO_PIN         5   // GPIO 5 (botão)

// Estados da sequência de LEDs
#define TODOS_LIGADOS     0
#define AZUL_LIGADO       1
#define VERMELHO_LIGADO   2
#define VERDE_LIGADO      3
#define TODOS_DESLIGADOS  4

int estado_atual = TODOS_DESLIGADOS; // Começa com todos os LEDs desligados
bool temporizador_ativo = false;     // Controla se o temporizador está ativo
struct repeating_timer timer;        // Temporizador

// Função de callback para o temporizador (retorna bool)
bool callback_temporizador(struct repeating_timer *t) {
    // Desliga todos os LEDs
    gpio_put(LED_VERMELHO_PIN, 0);
    gpio_put(LED_VERDE_PIN, 0);
    gpio_put(LED_AZUL_PIN, 0);

    // Atualiza o estado
    switch (estado_atual) {
        case TODOS_LIGADOS:
            estado_atual = AZUL_LIGADO; // Próximo estado: apenas azul ligado
            gpio_put(LED_AZUL_PIN, 1);  // Liga o azul
            printf("Mudança de estado: TODOS_LIGADOS -> AZUL_LIGADO\n");
            break;
        case AZUL_LIGADO:
            estado_atual = VERMELHO_LIGADO; // Próximo estado: apenas vermelho ligado
            gpio_put(LED_VERMELHO_PIN, 1);  // Liga o vermelho
            printf("Mudança de estado: AZUL_LIGADO -> VERMELHO_LIGADO\n");
            break;
        case VERMELHO_LIGADO:
            estado_atual = VERDE_LIGADO; // Próximo estado: apenas verde ligado
            gpio_put(LED_VERDE_PIN, 1);  // Liga o verde
            printf("Mudança de estado: VERMELHO_LIGADO -> VERDE_LIGADO\n");
            break;
        case VERDE_LIGADO:
            estado_atual = TODOS_DESLIGADOS; // Próximo estado: todos desligados
            temporizador_ativo = false;      // Desativa o temporizador após o ciclo completo
            printf("Mudança de estado: VERDE_LIGADO -> TODOS_DESLIGADOS\n");
            return false; // Cancela o temporizador
    }

    return true; // Continua executando o temporizador
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
    gpio_init(LED_VERDE_PIN);
    gpio_init(LED_AZUL_PIN);
    gpio_set_dir(LED_VERMELHO_PIN, GPIO_OUT);
    gpio_set_dir(LED_VERDE_PIN, GPIO_OUT);
    gpio_set_dir(LED_AZUL_PIN, GPIO_OUT);

    // Inicializar o pino do botão como entrada
    gpio_init(BOTAO_PIN);
    gpio_set_dir(BOTAO_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_PIN); // Ativar pull-up interno

    // Inicialmente, todos os LEDs desligados
    gpio_put(LED_VERMELHO_PIN, 0);
    gpio_put(LED_VERDE_PIN, 0);
    gpio_put(LED_AZUL_PIN, 0);

    printf("Estado inicial: TODOS_DESLIGADOS\n");

    // Loop principal
    while (true) {
        // Verifica se o botão foi pressionado e se o sistema está no estado TODOS_DESLIGADOS
        if (!gpio_get(BOTAO_PIN) && debounce() && estado_atual == TODOS_DESLIGADOS) {
            temporizador_ativo = true;

            // Liga todos os LEDs
            gpio_put(LED_VERMELHO_PIN, 1);
            gpio_put(LED_VERDE_PIN, 1);
            gpio_put(LED_AZUL_PIN, 1);

            // Define o estado inicial da sequência
            estado_atual = TODOS_LIGADOS;

            // Inicia o temporizador para mudar o estado a cada 3 segundos
            add_repeating_timer_ms(3000, callback_temporizador, NULL, &timer);
        }

        sleep_ms(100); // Pequeno atraso para evitar leitura excessiva do botão
    }

    return 0;
}