#include "driver/mcpwm.h"
#include "driver/gpio.h"

// Definição dos pinos GPIO
#define PHASE_A_H_GPIO 4   // Fase A High Side
#define PHASE_A_L_GPIO 5   // Fase A Low Side

#define PHASE_B_H_GPIO 18  // Fase B High Side
#define PHASE_B_L_GPIO 19  // Fase B Low Side

#define PHASE_C_H_GPIO 21  // Fase C High Side 
#define PHASE_C_L_GPIO 22  // Fase C Low Side 

// --- Definições Próprias para substituir a API nova ---
// 0: OFF (Floating/Low), 1: LOW (GND), 2: PWM
#define M_OFF 0
#define M_LOW 1
#define M_PWM 2

// Tabela de comutação 6 passos: {A_H, A_L, B_H, B_L, C_H, C_L}
// Lógica: 
// High Side (H): M_PWM (Ativa PWM) ou M_OFF (Desliga)
// Low Side (L):  M_LOW (Ativa terra) ou M_OFF (Desliga)
// Nota: Em drivers comuns, Low Side High = MOSFET ON (Terra conectado).
const int commutation_steps[6][6] = {
    // Passo 1: A+ (PWM), B- (LOW), C (Float)
    {M_PWM, M_OFF, M_OFF, M_LOW, M_OFF, M_OFF},
    // Passo 2: A+ (PWM), C- (LOW), B (Float)
    {M_PWM, M_OFF, M_OFF, M_OFF, M_OFF, M_LOW},
    // Passo 3: B+ (PWM), C- (LOW), A (Float)
    {M_OFF, M_OFF, M_PWM, M_OFF, M_OFF, M_LOW},
    // Passo 4: B+ (PWM), A- (LOW), C (Float)
    {M_OFF, M_LOW, M_PWM, M_OFF, M_OFF, M_OFF},
    // Passo 5: C+ (PWM), A- (LOW), B (Float)
    {M_OFF, M_LOW, M_OFF, M_OFF, M_PWM, M_OFF},
    // Passo 6: C+ (PWM), B- (LOW), A (Float)
    {M_OFF, M_OFF, M_OFF, M_LOW, M_PWM, M_OFF}
};

void set_phase(mcpwm_unit_t unit, mcpwm_timer_t timer, int state_h, int state_l, float duty) {
    // Configura o High Side (Operador A)
    if (state_h == M_PWM) {
        mcpwm_set_duty_type(unit, timer, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
        mcpwm_set_duty(unit, timer, MCPWM_OPR_A, duty);
    } else {
        // Desliga High Side (Geralmente sinal LOW desliga o MOSFET P ou N de cima se tiver driver)
        mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A); 
    }

    // Configura o Low Side (Operador B)
    if (state_l == M_LOW) {
        // Para ligar o MOSFET de baixo (conectar ao GND), enviamos sinal ALTO para o Gate
        mcpwm_set_signal_high(unit, timer, MCPWM_OPR_B);
    } else {
        // Desliga Low Side
        mcpwm_set_signal_low(unit, timer, MCPWM_OPR_B);
    }
}

void set_commutation_step(int step, float duty_cycle) {
    // Fase A (Timer 0)
    set_phase(MCPWM_UNIT_0, MCPWM_TIMER_0, commutation_steps[step][0], commutation_steps[step][1], duty_cycle);
    
    // Fase B (Timer 1)
    set_phase(MCPWM_UNIT_0, MCPWM_TIMER_1, commutation_steps[step][2], commutation_steps[step][3], duty_cycle);
    
    // Fase C (Timer 2)
    set_phase(MCPWM_UNIT_0, MCPWM_TIMER_2, commutation_steps[step][4], commutation_steps[step][5], duty_cycle);
}

void setup() {
    // Inicializa Serial para Debug
    Serial.begin(115200);

    // Mapeia GPIOs
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PHASE_A_H_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PHASE_A_L_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, PHASE_B_H_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, PHASE_B_L_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, PHASE_C_H_GPIO);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, PHASE_C_L_GPIO);

    // Configuração do MCPWM
    mcpwm_config_t cfg;
    cfg.frequency = 20000;    // 20 kHz
    cfg.cmpr_a = 0.0;
    cfg.cmpr_b = 0.0;
    cfg.counter_mode = MCPWM_UP_COUNTER;
    cfg.duty_mode = MCPWM_DUTY_MODE_0;

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &cfg);
 
    // No controle trapezoidal (6-step), controlamos manualmente quem liga/desliga.
    // O Deadtime automático do hardware pode interferir quando queremos deixar uma fase flutuando.
}

float duty_cycle = 30.0;
int step = 0;
int delay_us = 5000; // Começa devagar

void loop() {
    set_commutation_step(step, duty_cycle);

    step++;
    if (step >= 6) step = 0;

    delayMicroseconds(delay_us);

    // Ramp-up simples
    if (delay_us > 1000) {
        delay_us -= 5;
    }
}
