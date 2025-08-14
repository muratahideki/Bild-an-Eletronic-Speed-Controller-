#include "driver/mcpwm.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Definição dos pinos GPIO para as três fases (A, B, C)
#define PHASE_A_H_GPIO 4  // Fase A High Side
#define PHASE_A_L_GPIO 5  // Fase A Low Side

#define PHASE_B_H_GPIO 18 // Fase B High Side
#define PHASE_B_L_GPIO 19 // Fase B Low Side

#define PHASE_C_H_GPIO 35 // Fase C High Side
#define PHASE_C_L_GPIO 34 // Fase C Low Side

// Array para a sequência de comutação de 6 passos
// Cada linha representa um passo. As colunas são os estados das fases A, B e C.
// MCPWM_GEN_FORCED_HIGH: HIGH
// MCPWM_GEN_FORCED_LOW: LOW
// MCPWM_GEN_OFF: HIGH-Z (flutuante)
const mcpwm_action_t commutation_steps[6][6] = {
    // Passo 1: A+ B- C- (A+ B- C flutuante)
    {MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW},
    // Passo 2: A+ B flutuante C- (A+ C- B flutuante)
    {MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_HIGH},
    // Passo 3: A flutuante B+ C- (B+ C- A flutuante)
    {MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_OFF},
    // Passo 4: A- B+ C flutuante (B+ A- C flutuante)
    {MCPWM_GEN_OFF, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_FORCED_HIGH},
    // Passo 5: A- B flutuante C+ (C+ A- B flutuante)
    {MCPWM_GEN_FORCED_LOW, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_OFF},
    // Passo 6: A flutuante B- C+ (C+ B- A flutuante)
    {MCPWM_GEN_FORCED_LOW, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_FORCED_LOW}
};


void set_commutation_step(int step, float duty_cycle) {
    // Aplica o ciclo de trabalho e o estado de comutação para a fase A
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_action(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_FORCED_HIGH, commutation_steps[step][0]);
    mcpwm_set_action(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_GEN_FORCED_LOW, commutation_steps[step][1]);

    // Aplica o ciclo de trabalho e o estado de comutação para a fase B
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_action(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_FORCED_HIGH, commutation_steps[step][2]);
    mcpwm_set_action(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_GEN_FORCED_LOW, commutation_steps[step][3]);

    // Aplica o ciclo de trabalho e o estado de comutação para a fase C
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, duty_cycle);
    mcpwm_set_action(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_GEN_FORCED_HIGH, commutation_steps[step][4]);
    mcpwm_set_action(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_GEN_FORCED_LOW, commutation_steps[step][5]);
}

void app_main(void)
{
    // Mapeia GPIOs para MCPWM Unit 0, Timers 0, 1 e 2
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PHASE_A_H_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PHASE_A_L_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, PHASE_B_H_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, PHASE_B_L_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, PHASE_C_H_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, PHASE_C_L_GPIO);

    // Configuração do MCPWM (Frequência, ciclo de trabalho inicial, etc.)
    mcpwm_config_t cfg = {
        .frequency = 20000,          // 20 kHz (frequência típica para ESC)
        .cmpr_a    = 0.0,            // Ciclo de trabalho inicial de 0%
        .cmpr_b    = 0.0,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode    = MCPWM_DUTY_MODE_0,
    };

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &cfg);
    
    // Habilita o Dead-Time (tempo morto)
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 100, 100);
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 100, 100);
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 100, 100);

    // Loop principal para a comutação
    float duty_cycle = 50.0; // Ciclo de trabalho de 50%
    int step = 0;

    while (1) {
        // Aplica o passo de comutação atual
        set_commutation_step(step, duty_cycle);

        // Avança para o próximo passo
        step = (step + 1) % 6;

        // Aguarda um pequeno intervalo (aqui você leria os sensores Hall ou o Back-EMF)
        // O tempo de espera determina a velocidade do motor
        vTaskDelay(pdMS_TO_TICKS(100)); // Espera 100ms antes do próximo passo
    }
}
