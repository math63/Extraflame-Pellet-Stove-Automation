/**
* The MySensors Arduino library handles the wireless radio link and protocol
* between your home built sensors/actuators and HA controller of choice.
* The sensors forms a self healing radio network with optional repeaters. Each
* repeater and gateway builds a routing tables in EEPROM which keeps track of the
* network topology allowing messages to be routed to nodes.
*
* Created by Henrik Ekblad <henrik.ekblad@mysensors.org>
* Copyright (C) 2013-2015 Sensnology AB
* Full contributor list: https://github.com/mysensors/Arduino/graphs/contributors
*
* Documentation: http://www.mysensors.org
* Support Forum: http://forum.mysensors.org
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* version 2 as published by the Free Software Foundation.
*
*******************************
*
* DESCRIPTION
* The ArduinoGateway prints data received from sensors on the serial link.
* The gateway accepts input on seral which will be sent out on radio network.
*
* The GW code is designed for Arduino Nano 328p / 16MHz
*
* Wire connections (OPTIONAL):
* - Inclusion button should be connected between digital pin 3 and GND
* - RX/TX/ERR leds need to be connected between +5V (anode) and digital pin 6/5/4 with resistor 270-330R in a series
*
* LEDs (OPTIONAL):
* - To use the feature, uncomment any of the MY_DEFAULT_xx_LED_PINs
* - RX (green) - blink fast on radio message recieved. In inclusion mode will blink fast only on presentation recieved
* - TX (yellow) - blink fast on radio message transmitted. In inclusion mode will blink slowly
* - ERR (red) - fast blink on error during transmission error or recieve crc error
*
*/

// Enable debug prints to serial monitor
#define MY_DEBUG
#define MY_DEBUG_VERBOSE_RFM95

// Enable and select radio type attached
//#define MY_RADIO_NRF24
#define MY_RADIO_RFM95

#define MY_RFM95_CS_PIN 4 // NSS carte atmega1284p
#define MY_RFM95_IRQ_PIN 2 // IRQ D100
//#define MY_RFM95_RST_PIN 4

// Set LOW transmit power level as default, if you have an amplified NRF-module and
// power your radio separately with a good regulator you can turn up PA level.
//#define MY_RF24_PA_LEVEL RF24_PA_LOW

// Enable serial gateway
#define MY_GATEWAY_SERIAL

// Define a lower baud rate for Arduino's running on 8 MHz (Arduino Pro Mini 3.3V & SenseBender)
#if F_CPU == 8000000L
#define MY_BAUD_RATE 38400
#endif

// Enable inclusion mode
#define MY_INCLUSION_MODE_FEATURE
// Enable Inclusion mode button on gateway
//#define MY_INCLUSION_BUTTON_FEATURE

// Inverses behavior of inclusion button (if using external pullup)
//#define MY_INCLUSION_BUTTON_EXTERNAL_PULLUP

// Set inclusion mode duration (in seconds)
#define MY_INCLUSION_MODE_DURATION 60
// Digital pin used for inclusion mode button
//#define MY_INCLUSION_MODE_BUTTON_PIN  3

// Set blinking period
//#define MY_DEFAULT_LED_BLINK_PERIOD 300

// Inverses the behavior of leds
//#define MY_WITH_LEDS_BLINKING_INVERSE

// Flash leds on rx/tx/err
// Uncomment to override default HW configurations
//#define MY_DEFAULT_ERR_LED_PIN 4  // Error led pin
//#define MY_DEFAULT_RX_LED_PIN  6  // Receive led pin
//#define MY_DEFAULT_TX_LED_PIN  5  // the PCB, on board LED

#include <MySensors.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"

#define PELLET_NODE_ID  1

MyMessage msgGw2Node(0,V_CUSTOM);
//
//
MyMessage msgtempc(11,V_HVAC_SETPOINT_HEAT);
MyMessage msgpower(12,V_VAR3);
MyMessage msgprog(15,V_STATUS);
MyMessage msgstand(16,V_STATUS);
//
//
MyMessage msgtempa(20,V_TEMP);
MyMessage msgetat(21,V_TEXT);
MyMessage msgerreur(22,V_TEXT);
MyMessage msgtempfum(23,V_TEMP);
MyMessage msgtempres(24,V_TEMP);
MyMessage msgtempinair(25,V_TEMP);
MyMessage msgcadvis(26,V_VAR1);
MyMessage msgvitfum(27,V_VAR1);
MyMessage msgdurphase(28,V_VAR2);
MyMessage msgflux(29,V_VAR4);
MyMessage msgpowerreal(30,V_VAR3);

//
//
MyMessage temperatureMsg(40, V_TEMP);
MyMessage humidityMsg(41, V_HUM);
MyMessage pressureMsg(42, V_PRESSURE);
//                   0      1           2                         3                   4                     5           6                 7           8                         9               
//  String MsgEtat[]={"Off","Démarrage","Allumage - Charg pellet","Démarrage Flamme","Travail - Modulation","Nettoyage","Nettoyage Final","Stand-by","Erreur, Refroidissement","Erreur, Off"};
 const char* MsgEtat[]={"Off","Demarrage","Allumage-Charg","Dem Flamme","Travail-Modul","Nettoyage","Nettoyage Final","Stand-by","Erreur, Refroidissement","Erreur, Off"};
//                     0   1   2   3   4   5   6   7   8               9   10   11   12   13   14   15   16                 17   18
//  String MsgErreur[]={"-","1","2","3","4","5","6","7","Allumage Raté","9","10","11","12","13","14","15","Pellets terminés","17","18"};
 const char* MsgErreur[]={"-","No All Black-Out","Sonde Fumee","Temp Fumees","Asp Fumees KO","Allumage Rate","Pellets term","Depression","No Flux","Err inconnue"};

Adafruit_BME680 bme; // I2C

unsigned long delayTime;
  
void setup()
{
	// Setup locally attached sensors
 
  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }
  // weather monitoring
  
  // Set up oversampling and filter initialization
  
  bme.setTemperatureOversampling(BME680_OS_1X);
  bme.setHumidityOversampling(BME680_OS_1X);
  bme.setPressureOversampling(BME680_OS_1X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_0);  
  bme.setGasHeater(0, 0); // 320*C for 150 ms

  
                     
    // suggested rate is 1/60Hz (1m)
    delayTime = 60000; // in milliseconds
}

void presentation()
{
//  wait(30000);
sendSketchInfo("Pellet STOVE GW", "2.2");
  // Present locally attached sensors
  
//  present(10, S_HEATER); //commande  V_STATUS : on/off
//  present(11, S_HEATER); //commande V_HVAC_SETPOINT_HEAT : consigne temp
// present(12, S_HEATER); //commande V_VAR3 : consigne power
//  present(13, S_HEATER); //commande V_STATUS : actualisation
//  present(14, S_HEATER); //commande V_STATUS : Reset node
//  present(15, S_HEATER); //commande V_STATUS : prog
  
//  present(20, S_HEATER); //info V_TEMP : temp ambiant
//  present(21, S_HEATER); //info V_TEXT : Etat
//  present(22, S_HEATER); //info V_TEXT : Erreur
  
//    present(30, S_TEMP);
//    present(31, S_HUM);
//    present(32, S_BARO);
  
}

void loop()
{
  // Send locally attached sensor data here

  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
send(temperatureMsg.set(bme.temperature, 1));
send(humidityMsg.set(bme.humidity, 1));
send(pressureMsg.set(bme.pressure/100.0, 1));

wait(delayTime);
}

void receive(const MyMessage &message)
{
	if(message.sender == 0){
switch(message.sensor)
{	uint8_t datasent[2];
	case 10 : //on-off
	datasent[0]=1;
	datasent[1]=(uint8_t) message.getBool();
	send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
		break;
	case 11 : // consigne temp
	datasent[0]=2;
	datasent[1]=(uint8_t) message.getFloat();
	send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
		break;
	case 12 : // consigne power
	datasent[0]=3;
	datasent[1]=(uint8_t) message.getFloat();
	send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
		break;
	case 13 : // Actualisation
	datasent[0]=4;
	datasent[1]=(uint8_t) message.getBool();
	send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
		break;
	case 14 : // reset
	datasent[0]=5;
	datasent[1]=(uint8_t) message.getBool();
	send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
		break;
  case 15 : // Programmation on/off
  datasent[0]=6;
  datasent[1]=(uint8_t) message.getBool();
  send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
    break;
   case 16 : // Stand-by on-off
  datasent[0]=7;
  datasent[1]=(uint8_t) message.getBool();
  send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
    break;
  case 17 : // MAJ date/heures
  datasent[0]=8;
  datasent[1]=(uint8_t) message.getBool();
  send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
    break;
  case 18 : // Exploration RAM par lignes
  datasent[0]=9;
  datasent[1]=(uint8_t) message.getFloat();
  send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
    break;
  case 19 : // Exploration ROM par lignes
  datasent[0]=10;
  datasent[1]=(uint8_t) message.getFloat();
  send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
    break;
    
		
	default : 
	Serial.println("Erreur de commande");
	break;
	
}
} else if(message.sender == PELLET_NODE_ID && message.type== V_CUSTOM && message.sensor==1 ){
	
	uint8_t* data=(uint8_t*)message.getCustom();
	if (message.version_length>>3 == 13)
	{
		uint8_t tempc=0;
	  uint8_t tempa=0;
	  uint8_t power=0;
	  uint8_t etat=0;
	  uint8_t erreur=0;
    uint8_t prog=0;
    uint8_t stand=0;
    uint8_t temp_fum=0;
    uint8_t temp_res=0;
    uint8_t temp_inair=0;
    uint8_t cad_vis=0;
    uint8_t vit_fum=0;
    uint8_t dur_phase=0;
    uint8_t flux=0;
    uint8_t power_real=0;
 
	tempc =  data[0];
	tempa = data[1];
	power =  data[2]>>4 ;
	etat = data[2]& 0b00001111;
  prog = data[3]>>7;
  stand = (data[3]& 0b01000000)>>6 ;
	erreur = data[4];
  temp_fum = data[5];
	temp_res = data[6];
  temp_inair = data[7];
  cad_vis = data[8];
  vit_fum = data[9];
  dur_phase = data[10];
  flux = data[11];
  power_real = data[12];
  
	send(msgtempa.set(tempa/2.0,1));
	send(msgetat.set(MsgEtat[etat]));
	send(msgerreur.set(MsgErreur[transError(erreur)]));
	send(msgpower.set(power));
	send(msgtempc.set(tempc));
  send(msgprog.set(prog));
  send(msgstand.set(stand));
  send(msgtempfum.set(temp_fum));
  send(msgtempres.set(temp_res));
  send(msgtempinair.set(temp_inair/2.0-15.0,1));
  send(msgcadvis.set(cad_vis/40.0,1));
  send(msgvitfum.set(vit_fum*10+250));
  send(msgdurphase.set(dur_phase));
  send(msgflux.set(flux/10.0,1));
  send(msgpowerreal.set(power_real));
   
}
} else if(message.sender == PELLET_NODE_ID && message.type== V_CUSTOM && message.sensor==2 ){
  
	
} else if(message.sender == PELLET_NODE_ID && message.type== V_CUSTOM && message.sensor==3 ){
	
}
}

uint8_t transError(uint8_t erreur) {
  uint8_t error_tab;
  switch(erreur) {
  case 0 : // pas d'erreur
  error_tab= 0;
    break; 
  case 255 : // All black out
  error_tab= 1;
    break; 
  case 1 : // Sonde Fumee
  error_tab= 2;
    break; 
   case 2 : // Temp Fumees
  error_tab= 3;
    break;
  case 4 : // Asp Fumees KO
  error_tab= 4;
    break;
  case 8 : // Allumage Rate
  error_tab= 5;
    break;
  case 16 : // Pellets term
  error_tab= 6;
    break;
  case 32 : // Depression
  error_tab= 7;
   break;
  case 64 : // No Flux
  error_tab= 8;
    break;
  default : 
  error_tab = 9;
  break;
  }
  return error_tab;
}
