#include "driver/mcpwm.h"
#include "driver/adc.h"
#include "esp_timer.h"
#include "esp_log.h"

#define TAG "BLDC_SENSORLESS"

// --- Configuração ADC ---
#define ADC_CHANNEL ADC1_CHANNEL_0   // GPIO36 (exemplo, fase flutuante)
#define ADC_WIDTH   ADC_WIDTH_BIT_12
#define ADC_ATTEN   ADC_ATTEN_DB_11  // até ~3.6V

// --- Configuração PWM ---
#define PWM_FREQ    20000            // 20 kHz
#define PWM_UNIT    MCPWM_UNIT_0
#define PWM_TIMER   MCPWM_TIMER_0

// Duty inicial e máximo
int duty_cycle = 20;   // %
int delay_us   = 5000; // tempo inicial entre passos (ramp-up)

// --- Funções auxiliares ---
static inline void set_pwm_duty(mcpwm_unit_t unit, mcpwm_timer_t timer,
                                mcpwm_operator_t op, int duty_percent) {
    mcpwm_set_duty(unit, timer, op, duty_percent);
    mcpwm_set_duty_type(unit, timer, op, MCPWM_DUTY_MODE_0);
}

// --- Sequência de 6 passos ---
void set_commutation_step(int step, int duty) {
    switch (step) {
        case 0: // A+ B-
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_A, duty);
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_B, 0);
            mcpwm_set_signal_low(PWM_UNIT, PWM_TIMER, MCPWM_OPR_C);
            break;
        case 1: // A+ C-
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_A, duty);
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_C, 0);
            mcpwm_set_signal_low(PWM_UNIT, PWM_TIMER, MCPWM_OPR_B);
            break;
        case 2: // B+ C-
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_B, duty);
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_C, 0);
            mcpwm_set_signal_low(PWM_UNIT, PWM_TIMER, MCPWM_OPR_A);
            break;
        case 3: // B+ A-
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_B, duty);
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_A, 0);
            mcpwm_set_signal_low(PWM_UNIT, PWM_TIMER, MCPWM_OPR_C);
            break;
        case 4: // C+ A-
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_C, duty);
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_A, 0);
            mcpwm_set_signal_low(PWM_UNIT, PWM_TIMER, MCPWM_OPR_B);
            break;
        case 5: // C+ B-
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_C, duty);
            set_pwm_duty(PWM_UNIT, PWM_TIMER, MCPWM_OPR_B, 0);
            mcpwm_set_signal_low(PWM_UNIT, PWM_TIMER, MCPWM_OPR_A);
            break;
    }
}

// --- Leitura do ADC (fase flutuante) ---
int read_phase_voltage() {
    return adc1_get_raw(ADC_CHANNEL);
}

// --- Polling para detectar Zero Crossing ---
bool wait_for_zc(int vbus_half) {
    int last_val = read_phase_voltage();
    while (1) {
        int val = read_phase_voltage();
        // Detecta cruzamento em relação ao meio do barramento
        if ((last_val < vbus_half && val >= vbus_half) ||
            (last_val > vbus_half && val <= vbus_half)) {
            return true;
        }
        last_val = val;
    }
}

void app_main() {
    ESP_LOGI(TAG, "Iniciando BLDC Sensorless...");

    // Configuração PWM
    mcpwm_gpio_init(PWM_UNIT, MCPWM0A, 18); // Fase A
    mcpwm_gpio_init(PWM_UNIT, MCPWM0B, 19); // Fase B
    mcpwm_gpio_init(PWM_UNIT, MCPWM1A, 21); // Fase C

    mcpwm_config_t pwm_config = {
        .frequency = PWM_FREQ,
        .cmpr_a = 0,
        .cmpr_b = 0,
        .duty_mode = MCPWM_DUTY_MODE_0,
        .counter_mode = MCPWM_UP_COUNTER
    };
    mcpwm_init(PWM_UNIT, PWM_TIMER, &pwm_config);

    // Configuração ADC
    adc1_config_width(ADC_WIDTH);
    adc1_config_channel_atten(ADC_CHANNEL, ADC_ATTEN);

    int vbus_half = 2048; // ~VDD/2 em 12 bits
    int step = 0;

    // -----------------
    // 1. Ramp-up inicial
    // -----------------
    for (int i = 0; i < 200; i++) {  // 200 ciclos de aceleração
        set_commutation_step(step, duty_cycle);
        ets_delay_us(delay_us);
        step = (step + 1) % 6;

        if (delay_us > 1000) delay_us -= 20; // acelera
        if (duty_cycle < 60) duty_cycle++;   // aumenta torque
    }

    ESP_LOGI(TAG, "Entrando em modo ZC...");

    // -----------------
    // 2. Controle por ZC
    // -----------------
    while (1) {
        set_commutation_step(step, duty_cycle);

        // Aguarda cruzamento de zero
        if (wait_for_zc(vbus_half)) {
            // atraso de meio período até o próximo passo
            ets_delay_us(delay_us / 2);

            // próximo passo
            step = (step + 1) % 6;
        }
    }
}
