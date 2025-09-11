#include "driver/mcpwm.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TAG "ESC_SENSORLESS"

// Pinos para as fases (usar GPIOs compatíveis com PWM/ADC)
#define PHASE_A_H_GPIO 4
#define PHASE_A_L_GPIO 5
#define PHASE_B_H_GPIO 18
#define PHASE_B_L_GPIO 19
#define PHASE_C_H_GPIO 21
#define PHASE_C_L_GPIO 22

// Leitura de Back-EMF no ADC (usar o mesmo pino conectado via divisor resistivo!)
#define ADC_PHASE_A ADC1_CHANNEL_0   // GPIO36
#define ADC_PHASE_B ADC1_CHANNEL_3   // GPIO39
#define ADC_PHASE_C ADC1_CHANNEL_6   // GPIO34

// Sequência de comutação: {A+, A-, B+, B-, C+, C-}
const int comm_table[6][6] = {
    {1,0,0,1,0,0}, // A+ PWM, B- LOW, C flutuante
    {1,0,0,0,0,1}, // A+ PWM, C- LOW, B flutuante
    {0,0,1,0,0,1}, // B+ PWM, C- LOW, A flutuante
    {0,1,1,0,0,0}, // B+ PWM, A- LOW, C flutuante
    {0,1,0,0,1,0}, // C+ PWM, A- LOW, B flutuante
    {0,0,0,1,1,0}  // C+ PWM, B- LOW, A flutuante
};

// Qual fase fica flutuante em cada passo
const int floating_phase[6] = {2, 1, 0, 2, 1, 0};

int step = 0;
float duty_cycle = 30.0; // duty inicial

// Função: aplica comutação (mesmo que antes, simplificada aqui)
void set_commutation_step(int step, float duty_cycle) {
    // Exemplo: só mostra qual fase seria PWM/LOW/OFF
    ESP_LOGI(TAG, "Step %d: A+%d A-%d B+%d B-%d C+%d C-%d", step,
             comm_table[step][0], comm_table[step][1],
             comm_table[step][2], comm_table[step][3],
             comm_table[step][4], comm_table[step][5]);

    // Aqui entrariam as chamadas MCPWM como no código anterior
}

// Função: lê a fase flutuante pelo ADC
int read_floating_phase(int step) {
    int phase = floating_phase[step];
    int adc_raw = 0;

    switch(phase) {
        case 0: adc_raw = adc1_get_raw(ADC_PHASE_A); break;
        case 1: adc_raw = adc1_get_raw(ADC_PHASE_B); break;
        case 2: adc_raw = adc1_get_raw(ADC_PHASE_C); break;
    }

    return adc_raw;
}

void app_main(void) {
    // Configura ADC
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_PHASE_A, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_PHASE_B, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_PHASE_C, ADC_ATTEN_DB_11);

    // Inicializa MCPWM (igual ao código anterior)
    // ...

    ESP_LOGI(TAG, "Iniciando motor (open-loop startup)");

    // Etapa 1: Partida open-loop (dar alguns passos forçados até ganhar velocidade)
    for (int i = 0; i < 100; i++) {
        set_commutation_step(step, duty_cycle);
        step = (step + 1) % 6;
        vTaskDelay(pdMS_TO_TICKS(20)); // acelere gradualmente
    }

    ESP_LOGI(TAG, "Mudando para sensorless...");

    // Etapa 2: Rodar sensorless
    while (1) {
        // aplica passo atual
        set_commutation_step(step, duty_cycle);

        // espera até zero-crossing
        int midpoint = 2048; // ~ Vcc/2 para ADC 12 bits
        int adc_val = 0;
        int last_sign = 0;

        while (1) {
            adc_val = read_floating_phase(step);
            int sign = (adc_val > midpoint) ? 1 : -1;

            if (last_sign != 0 && sign != last_sign) {
                // detectou cruzamento!
                break;
            }
            last_sign = sign;
        }

        // espera meio período do elétrico
        vTaskDelay(pdMS_TO_TICKS(1)); // ajustar dinamicamente pela velocidade

        // avança passo
        step = (step + 1) % 6;
    }
}
