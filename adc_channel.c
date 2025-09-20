#include "driver/mcpwm.h"
#include "driver/adc.h"
#include "esp_timer.h"
#include "esp_log.h"


// --- Configuração ADC ---
#define ADC1 ADC1_CHANNEL_3   // GPIO4 (exemplo, fase flutuante)
#define ADC2 ADC1_CHANNEL_4   // GPIO5 (exemplo, fase flutuante)
#define ADC3 ADC2_CHANNEL_6   // GPIO17 (exemplo, fase flutuante)
#define ADC4 ADC2_CHANNEL_7   // GPIO18 (exemplo, fase flutuante)
#define ADC5 ADC2_CHANNEL_8   // GPIO19 (exemplo, fase flutuante)
#define ADC6 ADC2_CHANNEL_9   // GPIO20 (exemplo, fase flutuante)


#define ADC_WIDTH   ADC_WIDTH_BIT_12
#define ADC_ATTEN   ADC_ATTEN_DB_11  // até ~3.6V

int steps_adc[6] { ADC1, ADC2, ADC3, ADC4, ADC5, ADC6 };
int vbus = 12
int vbus_half = vbus / 2

std::array<int,6> read_ADC() {
  return {adc1_get_raw(ADC1),adc1_get_raw(ADC2),adc2_get_raw(ADC3),adc2_get_raw(ADC4),adc2_get_raw(ADC5),adc2_get_raw(ADC6)
    };
    
void comparator(){
  while(true) {
    auto adc = read_ADC()
    if (adc[0] < 100 && adc[1] < 100) {
          // return step = 4 ou 1
            };
  
    if (adc[2] < 100 && adc[3] < 100) {
          // return step = 3 ou 6
            };
  
    if (adc[4] < 100 && adc[5] < 100) {
          // return step = 2 ou 5
            };

// --- Polling para detectar Zero Crossing ---
int wait_for_ZC(int vbus_half){
  auto last_value = read_ADC();
  while (true) { 
    void comparator() 
  };



