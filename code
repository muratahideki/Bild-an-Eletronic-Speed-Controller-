#include "driver/mcpwm.h"
#include "driver/gpio.h"

#define f_PWM_GPIO_HIGH 4 // Pino onde o sinal PWM será gerado - high
#define f_PWM_GPIO_LOW 5 // Low

#define s_PWM_GPIO_HIGH 18 // High
#define s_PWM_GPIO_LOW 19 // Low

#define t_PWM_GPIO_HIGH 35 // High
#define t_PWM_GPIO_LOW 34 // Low 

void app_main(void)
{
    // Mapeia GPIOs para MCPWM Unit 0, Timer 0, canais A e B
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, f_PWM_GPIO_HIGH);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, f_PWM_GPIO_LOW);

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, s_PWM_GPIO_HIGH);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, s_PWM_GPIO_LOW);

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, t_PWM_GPIO_HIGH);
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, t_PWM_GPIO_LOW);

    // Uma única config para o mesmo TIMER (gera A e B)
    mcpwm_config_t cfg = {
        .frequency = 1000,              // 1 kHz
        .cmpr_a   = 50.0,               // duty A 50%
        .cmpr_b   = 50.0,               // duty B 50% (vai ser invertido abaixo)
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode    = MCPWM_DUTY_MODE_0, // por padrão, ativo em nível alto
    };

    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &cfg);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &cfg);

    // HIN e LIN são complementares (mesmo duty, fases opostas).
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_B, MCPWM_DUTY_MODE_1);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM_OPR_B, MCPWM_DUTY_MODE_1);
    mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_2, MCPWM_OPR_B, MCPWM_DUTY_MODE_1);

}


