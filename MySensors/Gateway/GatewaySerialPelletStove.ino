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

#define MY_RFM95_SPI_CS 10 // NSS
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


#define PELLET_NODE_ID  1

MyMessage msgGw2Node(0,V_CUSTOM);
MyMessage msgNode2Gw(1,V_CUSTOM);
//
//
MyMessage msgtempc(11,V_HVAC_SETPOINT_HEAT);
MyMessage msgpower(12,V_VAR3);

//
//
MyMessage msgtempa(20,V_TEMP);
MyMessage msgetat(21,V_VAR2);
MyMessage msgerreur(22,V_VAR3);

void setup()
{
	// Setup locally attached sensors
}

void presentation()
{
//  wait(30000);
sendSketchInfo("Pellet STOVE GW", "1.0");
  // Present locally attached sensors
  present(10, S_HEATER); //commande  V_STATUS : on/off
  present(11, S_HEATER); //commande V_HVAC_SETPOINT_HEAT : consigne temp
  present(12, S_HEATER); //commande V_VAR1 : consigne power
  present(13, S_HEATER); //commande V_STATUS : actualisation
  present(14, S_HEATER); //commande V_STATUS : Reset node
  
  present(20, S_HEATER); //info V_TEMP : temp ambiant
  present(21, S_HEATER); //info V_VAR2 : Etat
  present(22, S_HEATER); //info V_VAR3 : Erreur
}

void loop()
{
  // Send locally attached sensor data here
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
	case 13 : // refresh
	datasent[0]=4;
	datasent[1]=(uint8_t) message.getBool();
	send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
		break;
	case 14 : // refresh
	datasent[0]=5;
	datasent[1]=(uint8_t) message.getBool();
	send(msgGw2Node.setDestination(PELLET_NODE_ID).set(datasent,2));
		break;	
		
	default : 
	Serial.println("Erreur de commande");
	break;
	
}
} else if(message.sender == PELLET_NODE_ID && message.type== V_CUSTOM){
	
	uint8_t* data=(uint8_t*)message.getCustom();
	if (message.version_length>>3 == 4)
	{
		uint8_t tempc=0;
	  uint8_t tempa=0;
	  uint8_t power=0;
	  uint8_t etat=0;
	  uint8_t erreur=0;
	  
	
	tempc =  data[0];
	tempa = data[1];
	power =  data[2]>>4 ;
	etat = data[2]& 0b00001111;
	erreur = data[3];
	
	send(msgtempa.set(tempa/2.0,1));
	send(msgetat.set(etat));
	send(msgerreur.set(erreur));
	send(msgpower.set(power));
	send(msgtempc.set(tempc));
	}
}
}