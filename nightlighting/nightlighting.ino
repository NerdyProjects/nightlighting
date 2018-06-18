// #define SERIALUSB 1
#ifdef SERIALUSB
#include <DigiCDC.h>
#endif

#include <avr/sleep.h>

#define PIN_L1 0
#define PIN_L2 1
#define PWM_L1_DARK 255
#define PWM_L2_DARK 50
#define PWM_L1_LIGHT 0
#define PWM_L2_LIGHT 0
#define PWM_L1_DARK_LOWBAT 0
#define PWM_L2_DARK_LOWBAT 1

#define ANALOG_BATTERY 1 /* P2 is A1 */
#define ANALOG_LDR 0 /* P5 is A0 */

#define LDR_THRESHOLD 512
#define LDR_HYSTERESIS 10

/*  ADC = Ubat / (10k/(10k+100k)) / 2540mV * 1024
 *  Accuracy: +/-5% (so set it rather a bit safer, also to include further µC energy usage
 */
#define BATTERY_THRESHOLD 12000UL
#define BATTERY_ADC_THRESHOLD ((BATTERY_THRESHOLD) * 1024UL / 27940UL)
#define BATTERY_HYSTERESIS 2    /* Hysteresis in ADC steps - ~30 mV per step */

/* filter coefficients: each filter part adds 1/(2^x), f0 ~ fs / (2*pi*2^x) so approximately 2^x samples define the time scale of the filter */
#define LDR_FILTER_COEF_LOG2 8
#define BATTERY_FILTER_COEF_LOG2 5
#define SAMPLE_TIME_MILLIS 200UL


static uint32_t ldrMean;
static uint32_t batteryMean;
static uint8_t ldrState = 0;
static uint8_t batteryEmpty = 0;

void setup() {
  #ifdef SERIALUSB
  SerialUSB.begin();
  SerialUSB.delay(5000);
  #endif
  pinMode(PIN_L1, OUTPUT);
  pinMode(PIN_L2, OUTPUT);
  ldrMean = ((uint32_t)getLDR()) << LDR_FILTER_COEF_LOG2;
  batteryMean = ((uint32_t)getBattery()) << BATTERY_FILTER_COEF_LOG2;
}

static uint16_t getAnalogWithReference(uint8_t channel, uint8_t reference)
{
  analogReference(reference);
  #ifdef SERIALUSB
  SerialUSB.refresh();
  #endif
  analogRead(channel);
  #ifdef SERIALUSB
  SerialUSB.refresh();
  #endif
  return analogRead(channel);
}

static uint16_t getLDR() {
  /* LDR is ratiometric vs VCC */
  return getAnalogWithReference(ANALOG_LDR, 0); /* ADC_Reference_VCC */
}

static uint16_t getBattery() {
  /* battery voltage needs a well known reference */
  return getAnalogWithReference(ANALOG_BATTERY, 6); /* ADC_Reference_Internal_2p56 */
}

void loop() {
  static unsigned long nextSample = millis();
  if (millis() < nextSample)
  {
    #ifndef SERIALUSB
    /* go to sleep. Digispark will wakeup approximately once every millisecond by the millis systemtimer.
     I did not check other system library implementations. */
    sleep_mode();
    #else
    SerialUSB.refresh();
    #endif
  } else {
    /* fixed point to decimal conversion for filter and usage arithmetic */
    uint16_t ldr = ldrMean >> LDR_FILTER_COEF_LOG2;
    uint16_t battery = batteryMean >> BATTERY_FILTER_COEF_LOG2;
    /* filter updates */
    ldrMean = ldrMean - ldr + getLDR();
    batteryMean = batteryMean - battery + getBattery();
    
    if(!ldrState && ldr > (LDR_THRESHOLD + LDR_HYSTERESIS)) {
      ldrState = 1;
    } else if(ldrState && ldr < (LDR_THRESHOLD - LDR_HYSTERESIS)) {
      ldrState = 0;
    }
    if(!batteryEmpty && battery < (BATTERY_ADC_THRESHOLD - BATTERY_HYSTERESIS)) {
      batteryEmpty = 1;
    } else if(batteryEmpty && battery > (BATTERY_ADC_THRESHOLD + BATTERY_HYSTERESIS)) {
      batteryEmpty = 0;
    }
    if(ldrState) {
      /* light */
      analogWrite(PIN_L1, PWM_L1_LIGHT);
      analogWrite(PIN_L2, PWM_L2_LIGHT);
    } else {
      if(batteryEmpty) {
        analogWrite(PIN_L1, PWM_L1_DARK_LOWBAT);
        analogWrite(PIN_L2, PWM_L2_DARK_LOWBAT);
      } else {
        analogWrite(PIN_L1, PWM_L1_DARK);
        analogWrite(PIN_L2, PWM_L2_DARK);
      }
    }
    #ifdef SERIALUSB
    SerialUSB.print(ldr);
    SerialUSB.print(" ");
    SerialUSB.println(battery);
    #endif
    nextSample += SAMPLE_TIME_MILLIS;
  }
  #ifdef SERIALUSB
  if(SerialUSB.available()) {
    SerialUSB.read();
  }
  #endif
}
