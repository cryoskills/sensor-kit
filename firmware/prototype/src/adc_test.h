#include <Arduino.h>
#include <Wire.h>
#include <cryo_adc.h>

/*
 Testing prototype sensor kit for Cryoskills course
 Get data from PT1000 temperature probe, and convert to temperature value
*/ 

// set up some variables that we'll use later
int16_t adc_reading; //raw reading from the probe   
float_t adc_voltage; //reading converted to voltage  
float_t adc_resistance; //voltage converted to resistanace 
float_t adc_temp;    //resistance converted to temp!
float_t V_1; //voltage on one side of the wheatstone bridge
float_t V_2; //voltage on the other side of the wheatstone bridge

// define some parameters we already know 
float_t R_1 = 2970;  //measured R1 resistance
float_t R_2 = 2960;  //measured R2 resistance
float_t R_3 = 995;  //measured R3 resistance
float_t V_in = 3.3;  //input voltage

//create probe object. Specify the pins we are connected to, and think about which gain and averaging values will give us the best result
ADCDifferential adc(ADCDifferential::INPUT_PIN_POS::A1_PIN, ADCDifferential::INPUT_PIN_NEG::A2_PIN, ADCDifferential::GAIN::GAIN_8X, ADCDifferential::AVERAGES::AVG_X1024);

void adc_setup() {
 // Serial.begin(115200); //open serial connection
  adc.begin(); //use a command from the cryo_adc library to initialise our probe
  pinMode(6, OUTPUT); //set pin 6 to output current to our circuit
}

void adc_loop() {
  digitalWrite(6, HIGH); //switch pin 6 on, so we have the voltage from the controller across our circuit 

  // enable adc to take measurement
  adc.enable();

  delay(50); // leave some time for things to happen

  // take reading
  adc_reading = adc.read();

  // disable adc
  adc.disable();
  
  digitalWrite(6, HIGH); //switch pin 6 off again

  // print out our reading to serial, so we can check it out
//  Serial.println("Reading:");
//  Serial.println(adc_reading);
  
  // convert reading to voltage
  // the range of readings is from 0 to 32768, and the range of voltages we can measure is 0 to 1
  adc_voltage = (adc_reading/32768.*1);

  // print out our voltage to serial, for even more excitement
  // n.b, remember the gain!
  // n.b.b, think about units, what will give you the most accuracy? Are we working with floats or integers?
//  Serial.println("Voltage (mV):");
//  Serial.println(adc_voltage*1000/8);

 // #pragma region //get temperature value
  //convert voltage to temperature. Cast your mind back to high school maths class...
  V_2 = V_in*(R_3/(R_3+R_1));  // find V_2 using the voltage division rule
  V_1 = V_2+adc_voltage/8; // find V_1 using our adc voltage (which is the difference between V_1 and V_2)
  adc_resistance = (V_1*R_1)/(V_in-V_1); //also using voltage division rule, and rearranging for the resistance we want
  adc_temp = (adc_resistance-1000)/3.86;  //convert from resistance to temperature using the calibration curve. Think about how best to approximate this relationship (there are a few ways!)
  //#pragma endregion 
  
  // finally, print out our temperature value! How warm is the lab?!? 
  Serial.println("Analog probe:");
  Serial.print("Temp: ");
  Serial.println(adc_temp);

  // wait one second, give the poor processor a little break  
  delay(1000);
}

