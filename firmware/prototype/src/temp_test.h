#include <Arduino.h>
#include <Wire.h>
#include <OneWire.h>
#include <DS18B20.h>
#include <SparkFun_ENS160.h>
#include <SparkFunBME280.h>

float_t temperature;

/*
 Testing prototype sensor kit for Cryoskills course
 Get data from DS18B20 temperature probe and Sparkfun Environmental Combo (BME280), output to serial monitor
*/ 

// init probe objects
DS18B20 ds(5); //nb this uses the 'Arduino name' of the pin (D5)
//BME280 bm; // this is connected to pins SCL and SDA

void temp_setup() {
  //open serial connection
  //Serial.begin(115200);
//  Wire.begin();
 // if (bm.beginI2C() == false) //Begin communication over I2C
  //{
   // Serial.println("Did you plug it in??");
    //while(1); //Freeze
//  }
}

void temp_loop() {
  
 //get temperature value from DS probe, print to serial  
  
  while (ds.selectNext()) {
    Serial.println("Digital probe:");
    Serial.print("Temp: ");
    temperature = ds.getTempC();
    Serial.println(temperature);
    Serial.println();
  }

// get environmental data from Sparkfun breakout, print to serial
//  Serial.println("Environmental combo: ");
//  Serial.print("Humidity: ");
//  Serial.print(bm.readFloatHumidity(), 0);
//  Serial.print(" Pressure: ");
//  Serial.print(bm.readFloatPressure(), 0);
//  Serial.print(" Alt: ");
//  Serial.print(bm.readFloatAltitudeMeters(), 1);
//  Serial.print(" Temp: ");
//  Serial.print(bm.readTempC(), 2);
//  Serial.println();
//  Serial.print("\n");

// wait 1 second
  delay(1000);
}


