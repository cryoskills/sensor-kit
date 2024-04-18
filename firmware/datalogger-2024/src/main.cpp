/* ---------------- INCLUDES ---------------- */
#include <Arduino.h>

#include "DS18B20.h"

#include "cryo_system.h"
#include "cryo_adc.h"
#include "cryo_radio.h"
#include "cryo_sleep.h"
#include "cryo_power.h"

/* ---------------- USER CONFIGURATION ---------------- */

/* Unique Sensor ID for radio packet */
#define SENSOR_ID 0xc5999999

/* Baud rate for serial communication */
#define BAUD_RATE 115200

/* Measurement interval for temperature in seconds */
#define MEASUREMENT_INTERVAL 5

/*
  Analogue temperature mode
  -------------------------
  Comment out the corresponding line below depending on whether you
  have the components for the Wheatstone bridge installed (R25, R26)
  or not.
*/
// Temperature mode for Wheastone bridge
// #define TEMP_MODE_WHEATSTONE
// Temperature mode for potential divider
#define TEMP_MODE_POTENTIAL

/* ---------------- PIN CONFIGURAITON ---------------- */
/* 
  DS18B20 (digital temperature sensor) Data
  -----------------------------------------
  Pin for the OneWire interface to the DS18B20 digital temperature sensor.
*/ 
#define PIN_DS18B20_DATA 5

/* 
  Wheatstone Bridge Enable
  ------------------------
  Switches the supply voltage to the potential divider/Wheatstone bridge
  circuit between 0V and 3V3.
*/  
#define PIN_WHEAT_EN 11

/* 
  Potential Divider / Wheatstone Bridge +ve Input
  -----------------------------------------------
  Analogue-to-digital converter input for the potential divider/Wheatstone 
  bridge circuit with the PT1000 resistive temperature detector.
*/
#define PIN_WHEAT_IN_POS A1

/*
  Wheatstone Bridge -ve Input
  ---------------------------
  Analogue-to-digital converter input for the fixed side of the Wheatstone
  bridge circuit.

  Only required if the full Wheatstone bridge is used.
*/
#define PIN_WHEAT_IN_NEG A2

/*
  Radio Configuration Pins
  ------------------------
  Allocates the pin for radio enable (_EN), interrupt (_IRQ) and SPI
  interface chip select (_CS).
*/
#define PIN_RADIO_EN 6
#define PIN_RADIO_IRQ 9 
#define PIN_RADIO_CS 10

/* ---------------- CONSTANT DEFINITIONS ---------------- */
/*
  Measured Resistances
  --------------------
  Store the measured resistances (to the nearest Ohm) for R24, R25, R26
*/
const uint16_t R_R24 = 3300;
const uint16_t R_R25 = 3300;
const uint16_t R_R26 = 1000;

/* Voltage Supply
   --------------
  Store the system voltge (nominally 3V3) in mV   
*/
const uint16_t V_VCC_mV = 3300;

/* ---------------- FUNCTION DELARATIONS ---------------- */

// Temperature Functions
void do_temperature_measurements();

// Main PT1000 function
float get_pt1000_temperature();
  // Subroutines depending on configuration
  float get_pt1000_temperature_wheatstone();
  float get_pt1000_temperature_potential();
  // Stores the resistance to temperature conversion routine
  // in a different function
  float pt1000_resistance_to_temperature(float resistance);

// Main DS18B20 function
float get_ds18b20_temperature();

/* ---------------- PROGRAM VARIABLES ---------------- */
PseudoRTC* my_rtc;

// Variables for temperature sensors
float_t temperature_ds18b20;
float_t temperature_pt1000;
int16_t adc_pt1000_raw;

// Class for DS18B20
DS18B20 my_ds18b20(PIN_DS18B20_DATA);
// store address of DS18B20 during setup call
uint64_t ds18b20_addr;

// - PT1000 analogue ADC if using Wheastone bridge
#ifdef TEMP_MODE_WHEATSTONE

ADCDifferential my_adc(
  ADCDifferential::INPUT_PIN_POS::A1_PIN,
  ADCDifferential::INPUT_PIN_NEG::A2_PIN,
  ADCDifferential::GAIN::GAIN_4X,
  ADCDifferential::AVERAGES::AVG_X256,
  ADCDifferential::RESOLUTION::RES_AVG,
  ADCDifferential::VOLTAGE_REFERENCE::INT1V_INTERNAL
);


/*
  ADC Voltage Reference
  ---------------------
  We use the default value for the ADCDifferential class of 1V0 
  however you could experiment with changing this - you will just
  need to update the value below to correspond to the valid numeric
  equivalent.

  For example:
    ADCDifferential::VOLTAGE_REFERENCE::INTVCC1_INTERNAL
    corresponds to 0.5 * VDDANA (or 3.3V in our system)
    so the correct value of ADC_VOLTAGE_REFERENCE would be
    0.5 * 3.3 * 1000 = 1650
*/
const uint16_t ADC_VOLTAGE_REFERENCE = 1000;

#endif

// Select the debug level we are using - comment out this line
// to disable debugging!
extern CRYO_DEBUG_LEVEL CRYO_DEBUG;

/* ---------------- MAIN PROGRAM ---------------- */

void setup() {

  // Define debug type
  CRYO_DEBUG = DEBUG_SERIAL_AND_SD;

  // Initialise serial communication
  Serial.begin(BAUD_RATE);
  SerialDebug.begin(BAUD_RATE);

  /* ---------------- CONFIGURE PINS ---------------- */
  // Configure analogue input if using potential divider 
  #ifdef TEMP_MODE_POTENTIAL
    pinMode(PIN_WHEAT_IN_POS, INPUT);
  #endif

  /* ---------------- CONFIGURE RTC ---------------- */
  CRYO_DEBUG_MESSAGE("Configuring clock");
  cryo_configure_clock(__DATE__, __TIME__);

  CRYO_DEBUG_MESSAGE("Getting real-time clock");
  my_rtc = cryo_get_rtc();

  /* ---------------- INIT RADIO ---------------- */
  CRYO_DEBUG_MESSAGE("Initialising radio");
  if (!cryo_radio_init(SENSOR_ID, my_rtc)) {
    CRYO_DEBUG_MESSAGE("Failed to initialise radio");
    cryo_error(CRYO_ERROR_RADIO_INIT);
  }

  /* ---------------- SETUP SD CARD ---------------- */

  // TODO: Move this code into a cryo_init() function or similar
  if (CRYO_DEBUG == CRYO_DEBUG_LEVEL::DEBUG_SERIAL_AND_SD)
    cryo_add_alarm_every(CRYO_DEBUG_SD_FLUSH, _cryo_debug_sd_flush); 

  /* ---------------- SETUP PT1000 / ADC ---------------- */
  #ifdef TEMP_MODE_WHEATSTONE
    my_adc.begin();
  #endif
  pinMode(PIN_WHEAT_EN, OUTPUT);

  /* ---------------- SETUP DS18B20 ---------------- */
  if (!my_ds18b20.selectNext()) {
    CRYO_DEBUG_MESSAGE("Failed to initialise DS18B20 sensor");
    cryo_error(CRYO_ERROR_DS18B20_INIT);
  }
  // Set DS18b20 to highest resolution
  my_ds18b20.setResolution(12);

  // Reset PT1000 and DS18b20 variables
  temperature_ds18b20 = NAN;
  temperature_pt1000 = NAN;

  /* ---------------- ADD MEASUREMENT ALARM ---------------- */
  cryo_add_alarm_every(MEASUREMENT_INTERVAL, do_temperature_measurements);
  
}

void loop() {
  // put your main code here, to run repeatedly:
  cryo_wakeup();
  // Serial.begin(BAUD_RATE);
  // SerialDebug.begin(BAUD_RATE);
  cryo_raise_alarms();
  cryo_sleep();
}

void do_temperature_measurements() {
  digitalWrite(LED_BUILTIN, HIGH);
  CRYO_DEBUG_MESSAGE("Taking measurements");
  CRYO_DEBUG_MESSAGE("Doing digital temperature measurement");
  temperature_ds18b20 = get_ds18b20_temperature();
  CRYO_DEBUG_MESSAGE("Doing analogue temperature measurement");
  temperature_pt1000 = get_pt1000_temperature(); 
  char raw_message[32];
  sprintf(raw_message, "Digital  : %d mC", (int)(temperature_ds18b20*1000));
  CRYO_DEBUG_MESSAGE(raw_message);
  sprintf(raw_message, "Analogue : %d mC", (int)(temperature_pt1000*1000));
  CRYO_DEBUG_MESSAGE(raw_message);
  digitalWrite(LED_BUILTIN, LOW);
}

float_t get_ds18b20_temperature() {

  // Get next DS18B20 temperature sensor
  while (my_ds18b20.selectNext()) {
    // and return value when we have it
    return my_ds18b20.getTempC();
  }

}

float_t get_pt1000_temperature() {
  
  #ifdef TEMP_MODE_WHEATSTONE
    // Code for Wheatstone bridge
    return get_pt1000_temperature_wheatstone();
  #else
    // Code for Potential divider
    return get_pt1000_temperature_potential();
  #endif

}

float_t get_pt1000_temperature_wheatstone() {

#ifdef TEMP_MODE_WHEATSTONE
  CRYO_DEBUG_MESSAGE("Enabling voltage at potential divider");
  // Enable output to potential divider 
  digitalWrite(PIN_WHEAT_EN, HIGH);
  // Enable the ADC
  my_adc.enable();
  // Allow some settling time at the pin to compensate for any parasitic
  // capacitances
  delay(100);

  // Get result from ADC
  CRYO_DEBUG_MESSAGE("Reading ADC");
  adc_pt1000_raw = my_adc.read();
  // Get the configured gain of the ADC
  CRYO_DEBUG_MESSAGE("Writing value to string");
  char raw_message[32];
  sprintf(raw_message, "Raw ADC: %d", adc_pt1000_raw);
  CRYO_DEBUG_MESSAGE(raw_message);
  
  CRYO_DEBUG_MESSAGE("Converting gain");
  float_t gain = my_adc.get_gain_numeric();
  
  // CAREFUL - We use the constant numeric voltage reference from the definitions above
  //           change this value if you modify the ADC configuration!
  float_t voltage = ((float_t) adc_pt1000_raw) * ADC_VOLTAGE_REFERENCE / gain / 32768;
  sprintf(raw_message, "Calculate ADC voltage: %d mV", (int)voltage);
  CRYO_DEBUG_MESSAGE(raw_message);

  // Calculate RTD (+ve) voltage
  float_t voltage_pos = V_VCC_mV * R_R26 / (R_R25 + R_R26) + voltage;

  // Calculate RTD resistance
  float_t r_pt1000 = voltage_pos / (V_VCC_mV - voltage_pos) * R_R24;

  // digitalWrite(PIN_WHEAT_EN, LOW);
  my_adc.disable();
  
  // Convert to temperature
  return pt1000_resistance_to_temperature(r_pt1000);
#else
  return 0;
#endif

}

float_t get_pt1000_temperature_potential() {

#ifdef TEMP_MODE_POTENTIAL
  CRYO_DEBUG_MESSAGE("Enabling voltage at potential divider");
  // Enable output to potential divider 
  digitalWrite(PIN_WHEAT_EN, HIGH);
  // Allow some settling time at the pin to compensate for any parasitic
  // capacitances
  delay(100);

  // Get result from ADC
  CRYO_DEBUG_MESSAGE("Reading ADC");
  adc_pt1000_raw = analogRead(PIN_WHEAT_IN_POS);
  // Get the configured gain of the ADC
  CRYO_DEBUG_MESSAGE("Writing value to string");
  char raw_message[32];
  sprintf(raw_message, "Raw ADC: %d", adc_pt1000_raw);
  CRYO_DEBUG_MESSAGE(raw_message);
  
  // CAREFUL - We use the constant numeric voltage reference from the definitions above
  //           change this value if you modify the ADC configuration!
  float_t voltage = ((float_t) adc_pt1000_raw) * V_VCC_mV / 1023;
  sprintf(raw_message, "Calculated ADC voltage: %d mV", (int)voltage);
  CRYO_DEBUG_MESSAGE(raw_message);

  // Calculate RTD resistance
  float_t r_pt1000 = voltage / (V_VCC_mV - voltage) * R_R24;

  // digitalWrite(PIN_WHEAT_EN, LOW);
  
  // Convert to temperature
  return pt1000_resistance_to_temperature(r_pt1000);
#else
  return 0;
#endif

}

float_t pt1000_resistance_to_temperature(float_t resistance) {
  return (resistance - 1000) / 3.98;
}