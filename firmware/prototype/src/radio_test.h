// rf95_client.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing client
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95 if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf95_server
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with
// the RFM95W, Adafruit Feather M0 with RFM95
 
#include <SPI.h>
#include <RH_RF95.h>

// set this to your unique instrument ID
#define DEVICE_ID 0xc501;

extern float_t temperature;
uint8_t data_packet[6];
uint16_t instrument_id;
 
// Singleton instance of the radio driver
//RH_RF95 rf95;
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
RH_RF95 rf95(10, 9); // Adafruit Feather M0 with RFM95 
 
// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB
 
void radio_setup() 
{
    // reset
//  pinMode(11,OUTPUT);
//  digitalWrite(11, HIGH);
//  delay(1);
//  pinMode(11,OUTPUT);
//  digitalWrite(11, LOW);
//  delay(1);
//  pinMode(11,OUTPUT);
//  digitalWrite(11, HIGH);
  instrument_id = DEVICE_ID;
  
  Serial.begin(9600);
  while (!Serial) ; // Wait for serial port to be available
  if (!rf95.init())
    Serial.println("init failed");
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on
 
  // You can change the modulation parameters with eg
  // rf95.setModemConfig(RH_RF95::Bw500Cr45Sf128);
  
  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 2 to 20 dBm:
//  rf95.setTxPower(20, false);
  // If you are using Modtronix inAir4 or inAir9, or any other module which uses the
  // transmitter RFO pins and not the PA_BOOST pins
  // then you can configure the power transmitter power for 0 to 15 dBm and with useRFO true. 
  // Failure to do that will result in extremely low transmit powers.
//  rf95.setTxPower(14, true);
}
 
void radio_loop()
{
  // package up instrument ID and temperature value to send
  memcpy(data_packet, &instrument_id, 2);
  memcpy(data_packet+2, &temperature, 4);

  
  Serial.println("Sending to rf95_server");
  // Send a message to rf95_server
  //uint8_t data[] = "Hello World!";
  //rf95.send(data, sizeof(data));
  Serial.printf("%x,%x,%x,%x,%x,%x,\n", data_packet[0], data_packet[1],data_packet[2],data_packet[3],data_packet[4],data_packet[5]);
  rf95.send(data_packet, sizeof(data_packet));
  
  rf95.waitPacketSent();
  // Now wait for a reply
  uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
  uint8_t len = sizeof(buf);
 
  if (rf95.waitAvailableTimeout(3000))
  { 
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      Serial.print("got reply: ");
      Serial.println((char*)buf);
//      Serial.print("RSSI: ");
//      Serial.println(rf95.lastRssi(), DEC);    
    }
    else
    {
      Serial.println("recv failed");
    }
  }
  else
  {
    Serial.println("No reply, is rf95_server running?");
  }
  delay(400);
}