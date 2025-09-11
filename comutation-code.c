#include "driver/mcpwm.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Definição dos pinos GPIO para as três fases (A, B, C)
#define PHASE_A_H_GPIO 4   // Fase A High Side
#define PHASE_A_L_GPIO 5   // Fase A Low Side

#define PHASE_B_H_GPIO 18  // Fase B High Side
#define PHASE_B_L_GPIO 19  // Fase B Low Side

#define PHASE_C_H_GPIO 21  // Fase C High Side 
#define PHASE_C_L_GPIO 22  // Fase C Low Side 

// Tabela de 6 passos: {A+, A-, B+, B-, C+, C-}
// PWM → MCPWM_GEN_FORCED_HIGH
// LOW → MCPWM_GEN_FORCED_LOW
// OFF → MCPWM_GEN_OFF
const mcpwm_action_t commutation_steps[6][6] = {
    // Passo 1: A+ PWM, B- LOW, C Z
    {MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_OFF, MCPWM_GEN_OFF},
    // Passo 2: A+ PWM, C- LOW, B Z
    {MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW},
    // Passo 3: B+ PWM, C- LOW, A Z
    {MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW},
    // Passo 4: B+ PWM, A- LOW, C Z
    {MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_OFF},
    // Passo 5: C+ PWM, A- LOW, B Z
    {MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_OFF},
    // Passo 6: C+ PWM, B- LOW, A Z
    {MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_OFF, MCPWM_GEN_FORCED_LOW, MCPWM_GEN_FORCED_HIGH, MCPWM_GEN_OFF}
};

// Função para aplicar o passo de comutação
void set_commutation_step(int step, float duty_cycle) {
    // --- Fase A ---
    if (commutation_steps[step][0] == MCPWM_GEN_FORCED_HIGH) {
        // A+ com PWM
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, duty_cycle);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    } else if (commutation_steps[step][1] == MCPWM_GEN_FORCED_LOW) {
        // A- aterrado
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    } else {
        // A flutuante
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B);
    }

    // --- Fase B ---
    if (commutation_steps[step][2] == MCPWM_GEN_FORCED_HIGH) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, duty_cycle);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    } else if (commutation_steps[step][3] == MCPWM_GEN_FORCED_LOW) {
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B);
    } else {
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_A);
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B);
    }

    // --- Fase C ---
    if (commutation_steps[step][4] == MCPWM_GEN_FORCED_HIGH) {
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, duty_cycle);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
    } else if (commutation_steps[step][5] == MCPWM_GEN_FORCED_LOW) {
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B);
    } else {
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_A);
        mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B);
    }
}

void app_main(void) {
    // Mapeia GPIOs para MCPWM
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PHASE_A_H_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PHASE_A_L_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, PHASE_B_H_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, PHASE_B_L_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, PHASE_C_H_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, PHASE_C_L_GPIO);

    // Configuração do MCPWM
    mcpwm_config_t cfg = {
        .frequency = 20000, // 20 kHz (frequência típica de ESC)
        .cmpr_a = 0.0,
        .cmpr_b = 0.0,
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &cfg);

    // Habilita dead-time para proteger MOSFETs
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 100, 100);
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 100, 100);
    mcpwm_deadtime_enable(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_ACTIVE_HIGH_COMPLIMENT_MODE, 100, 100);

    // Loop principal de comutação
    float duty_cycle = 50.0; // Ciclo de trabalho em %
    int step = 0;

    while (1) {
        set_commutation_step(step, duty_cycle);

        step = (step + 1) % 6; // Avança para o próximo passo

        vTaskDelay(pdMS_TO_TICKS(50)); // ~20 Hz → só para teste (motor vai dar trancos)
    }
}
