#include "driver/mcpwm.h"
#include "driver/adc.h"
#include "esp_timer.h"
#include "esp_log.h"


// --- Configuração ADC ---
#define ADC_CHANNEL ADC1_CHANNEL_3   // GPIO4 (exemplo, fase flutuante)
#define ADC_CHANNEL ADC1_CHANNEL_4   // GPIO5 (exemplo, fase flutuante)
#define ADC_CHANNEL ADC2_CHANNEL_6   // GPIO17 (exemplo, fase flutuante)
#define ADC_CHANNEL ADC2_CHANNEL_7   // GPIO18 (exemplo, fase flutuante)
#define ADC_CHANNEL ADC2_CHANNEL_8   // GPIO19 (exemplo, fase flutuante)
#define ADC_CHANNEL ADC2_CHANNEL_9   // GPIO20 (exemplo, fase flutuante)


#define ADC_WIDTH   ADC_WIDTH_BIT_12
#define ADC_ATTEN   ADC_ATTEN_DB_11  // até ~3.6V

int steps
