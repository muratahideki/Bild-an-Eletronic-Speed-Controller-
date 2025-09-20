#include <array>
#include "driver/adc.h"
#include "esp_log.h"

#define ADC_WIDTH   ADC_WIDTH_BIT_12
#define ADC_ATTEN   ADC_ATTEN_DB_11  // até ~3.6 V

#define PHASE_A ADC1_CHANNEL_3
#define PHASE_B ADC1_CHANNEL_4
#define PHASE_C ADC1_CHANNEL_5
#define PHASE_D ADC1_CHANNEL_6
#define PHASE_E ADC1_CHANNEL_7
#define PHASE_F ADC1_CHANNEL_0 


int steps_adc[6] { 1,2,3,4,5,6 }; // verificar se é int msm essa função

int vbus = 12;
int vbus_half = vbus / 2;

std::array<int,6> read_ADC() {
    return {
        adc1_get_raw(PHASE_A),
        adc1_get_raw(PHASE_B),
        adc1_get_raw(PHASE_C),
        adc1_get_raw(PHASE_D),
        adc1_get_raw(PHASE_E),
        adc1_get_raw(PHASE_F)
    };
}
    
int comparator(int vbus_half, int steps_adc){
  while(true) {
    auto adc_latest = read_ADC();
    // return step = 4 ou 1
    if (adc[0] < 100 && adc[1] < 100) {
      auto adc_a = read_ADC();
      //cruzando para cima 
      if ( adc_latest[0] < vbus_half && adc_a[0] >= vbus_half ){
        return steps_adc[1] 
      };
      adc_latest[0] = adc_a[0];
      //cruzando para baixo 
      elif ( adc_latest[1] >= vbus_half && adc_a[1] < vbus_half){
        return steps_adc[4]
      };     
      adc_latest[1] = adc_a[1];
    };

    if (adc[2] < 100 && adc[3] < 100) {
      auto adc_b = read_ADC()
        if ( adc_latest[2] < vbus_half && adc_b[2] >= vbus_half){
          return steps_adc[3] 
        };
        adc_latest[2] = adc_b[2];

        elif ( adc_latest[3] >= vbus_half && adc_b[3] < vbus_half){
          return steps_adc[6]};
        };
        adc_latest[3] = adc_b[3];
    };

    if (adc[4] < 100 && adc[5] < 100 ) {
      auto adc = read_ADC()
        if ( adc_latest[4] < vbus_half && adc[4] >= vbus_half){
          return steps_adc[5] 
        };
        adc_latest[4] = adc[4];
        elif ( adc_latest[5] >= vbus_half && adc[5] < vbus_half){
          return steps_adc[2]
        };
        adc_latest[5] = adc[5];
    };
  };

// --- Polling para detectar Zero Crossing ---
int wait_for_ZC(int vbus_half){
  auto last_value = read_ADC();
  while (true) { 
    void comparator() 
  };
};



