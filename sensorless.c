#include "driver/mcpwm.h"
#include "driver/gpio.h"
#include "driver/adc.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// --- Definições de Hardware ---
// Saídas para MOSFETs
#define PHASE_A_H 4
#define PHASE_A_L 5
#define PHASE_B_H 18
#define PHASE_B_L 19
#define PHASE_C_H 21
#define PHASE_C_L 22

// Entradas de Leitura de Tensão (ADCs após divisor de tensão)
// ADC1 é preferível pois ADC2 conflita com WiFi
#define ADC_A ADC1_CHANNEL_0  // GPIO 36 (VP)
#define ADC_B ADC1_CHANNEL_3  // GPIO 39 (VN)
#define ADC_C ADC1_CHANNEL_6  // GPIO 34

// Limiar do Zero Cross (Metade da tensão lida no ADC)
// Se o ADC lê até 4095 (3.3V), e seu divisor coloca o "centro" virtual em 1.65V
#define ZC_THRESHOLD 1800 

typedef enum { PHASE_OFF, PHASE_PWM, PHASE_LOW } phase_state_t;
typedef enum { RISING, FALLING } edge_direction_t;

// Estrutura expandida para saber quem ler
typedef struct {
    phase_state_t a;
    phase_state_t b;
    phase_state_t c;
    adc1_channel_t floating_adc; // Qual fase ler?
    edge_direction_t edge;       // Esperamos que suba ou desça?
} step_config_t;

// Tabela de 6 passos com info de BEMF
const step_config_t steps[6] = {
    // Passo 1: A+, B-, C(Floating). C estava em Low, então vai subir (Rising)
    {PHASE_PWM, PHASE_LOW, PHASE_OFF, ADC_C, RISING}, 
    // Passo 2: A+, C-, B(Floating). B estava em Low, vai subir
    {PHASE_PWM, PHASE_OFF, PHASE_LOW, ADC_B, RISING},
    // Passo 3: B+, C-, A(Floating). A estava em High, vai descer (Falling)
    {PHASE_OFF, PHASE_PWM, PHASE_LOW, ADC_A, FALLING},
    // Passo 4: B+, A-, C(Floating). C estava em High, vai descer
    {PHASE_LOW, PHASE_PWM, PHASE_OFF, ADC_C, FALLING},
    // Passo 5: C+, A-, B(Floating). B estava em High, vai descer
    {PHASE_LOW, PHASE_OFF, PHASE_PWM, ADC_B, FALLING},
    // Passo 6: C+, B-, A(Floating). A estava em Low, vai subir
    {PHASE_OFF, PHASE_LOW, PHASE_PWM, ADC_A, RISING}
};

void setup_mcpwm() {
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PHASE_A_H);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, PHASE_A_L);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, PHASE_B_H);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, PHASE_B_L);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, PHASE_C_H);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, PHASE_C_L);

    mcpwm_config_t cfg = {
        .frequency = 20000, .cmpr_a = 0, .cmpr_b = 0,
        .counter_mode = MCPWM_UP_COUNTER, .duty_mode = MCPWM_DUTY_MODE_0
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &cfg);
}

void setup_adc() {
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC_A, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_B, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(ADC_C, ADC_ATTEN_DB_11);
}

void set_phase(mcpwm_unit_t unit, mcpwm_timer_t timer, phase_state_t state, float duty) {
    if (state == PHASE_PWM) {
        mcpwm_set_duty(unit, timer, MCPWM_OPR_A, duty);
        mcpwm_set_duty_type(unit, timer, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
        mcpwm_set_signal_low(unit, timer, MCPWM_OPR_B);
    } else if (state == PHASE_LOW) {
        mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A);
        mcpwm_set_signal_high(unit, timer, MCPWM_OPR_B);
    } else { // OFF / Floating
        mcpwm_set_signal_low(unit, timer, MCPWM_OPR_A);
        mcpwm_set_signal_low(unit, timer, MCPWM_OPR_B);
    }
}

void apply_step(int step_idx, float duty) {
    set_phase(MCPWM_UNIT_0, MCPWM_TIMER_0, steps[step_idx].a, duty);
    set_phase(MCPWM_UNIT_0, MCPWM_TIMER_1, steps[step_idx].b, duty);
    set_phase(MCPWM_UNIT_0, MCPWM_TIMER_2, steps[step_idx].c, duty);
}

void app_main(void) {
    setup_mcpwm();
    setup_adc();

    int step = 0;
    float duty = 20.0;
    uint32_t step_delay = 5000; // Começa lento (Open Loop)
    bool closed_loop = false;
    
    // Variáveis para cálculo de tempo
    int64_t last_step_time = esp_timer_get_time();
    int64_t zero_cross_time = 0;

    while (1) {
        apply_step(step, duty);
        
        // Ignorar picos de comutação (Flyback masking)
        ets_delay_us(step_delay / 4); 

        // --- Lógica Sensorless ---
        if (closed_loop) {
            int reading = 0;
            bool zc_detected = false;
            int timeout = 10000; // Segurança para não travar

            while (!zc_detected && timeout > 0) {
                reading = adc1_get_raw(steps[step].floating_adc);
                
                if (steps[step].edge == RISING) {
                    if (reading > ZC_THRESHOLD) zc_detected = true;
                } else { // FALLING
                    if (reading < ZC_THRESHOLD) zc_detected = true;
                }
                timeout--;
                // Pequeno delay para permitir o ADC respirar
                ets_delay_us(10);
            }

            // Se detectou o Zero Cross:
            if (zc_detected) {
                 // O ZC acontece no meio do passo (30 graus elétricos).
                 // Precisamos esperar mais 30 graus para comutar.
                 // Simplificação: Espera metade do tempo do passo anterior.
                 ets_delay_us(step_delay / 2);
                 
                 // Atualiza o tempo do passo para o próximo ciclo
                 int64_t now = esp_timer_get_time();
                 step_delay = (now - last_step_time); // Auto-ajuste da velocidade
                 last_step_time = now;
            } else {
                // Perdeu a sincronia? Volte para Open Loop ou force comutação
                closed_loop = false; 
                duty = 20.0; // Reduz potência
            }

        } else {
            // --- Modo Open Loop (Partida) ---
            ets_delay_us(step_delay);
            
            // Rampa de Aceleração
            if (step_delay > 1000) {
                step_delay -= 20;
            } else {
                // Se atingiu velocidade suficiente, tenta engatar o Sensorless
                closed_loop = true;
                // Aumenta um pouco a força para manter o torque
                duty = 35.0; 
            }
            last_step_time = esp_timer_get_time();
        }

        step++;
        if (step >= 6) step = 0;
    }
}
