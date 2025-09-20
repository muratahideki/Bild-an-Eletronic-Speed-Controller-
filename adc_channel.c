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

int steps_adc[6] { 1,2,3,4,5,6 }; // verificar se é int msm essa função

int vbus = 12
int vbus_half = vbus / 2

std::array<int,6> read_ADC() {
  return {adc1_get_raw(ADC1),adc1_get_raw(ADC2),adc2_get_raw(ADC3),adc2_get_raw(ADC4),adc2_get_raw(ADC5),adc2_get_raw(ADC6)
    };
    
void comparator(int vbus_half, int steps_adc){
  while(true) {
    auto adc_latest = read_ADC();
    // return step = 4 ou 1
    if (adc[0,1] < 100) {
      auto adc[0,1] = read_ADC()
      //cruzando para cima 
      if ( adc_latest[0,1] < vbus_half && adc[0,1] >= vbus_half){
        return steps_adc[1] };
      //cruzando para baixo 
      elif ( adc_latest[0,1] >= vbus_half && adc[0,1] < vbus_half){
        return steps_adc[4]};     
        };
        adc_latest[0,1] = adc[0,1];
  
    if (adc[2,3] < 100) {
      auto adc[2,3] = read_ADC()
        if ( adc_latest[2,3] < vbus_half && adc[2,3] >= vbus_half){
          return steps_adc[3] };
        //cruzando para baixo 
        elif ( adc_latest[2,3] >= vbus_half && adc[2,3] < vbus_half){
          return steps_adc[6]};
        };
        adc_latest[2,3] = adc[2,3];

    if (adc[4,5] < 100) {
      auto adc[4,5] = read_ADC()
        if ( adc_latest[4,5] < vbus_half && adc[4,5] >= vbus_half){
          return steps_adc[5] };
        //cruzando para baixo 
        elif ( adc_latest[4,5] >= vbus_half && adc[4,5] < vbus_half){
          return steps_adc[2]};
        };
        adc_latest[4,5] = adc[4,5];
  };

// --- Polling para detectar Zero Crossing ---
int wait_for_ZC(int vbus_half){
  auto last_value = read_ADC();
  while (true) { 
    void comparator() 
  };



