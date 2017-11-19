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
 * REVISION HISTORY
 * Version 1.0 - Henrik Ekblad
 *
 * DESCRIPTION
 * Example sketch showing how to control physical relays.
 * This example will remember relay state after power failure.
 * http://www.mysensors.org/build/relay
 */

// Enable debug prints to serial monitor
#define MY_DEBUG
#define MY_DEBUG_VERBOSE_RFM95

// Enable and select radio type attached
//#define MY_RADIO_NRF24
//#define MY_RADIO_RFM69
#define MY_RADIO_RFM95

#define MY_RFM95_SPI_CS 10 // NSS
#define MY_RFM95_IRQ_PIN 2 // IRQ D100

// Enable repeater functionality for this node
//#define MY_REPEATER_FEATURE

#define MY_NODE_ID 1

#include <MySensors.h>
#include <single_wire_UART.h>
#include <avr/wdt.h>

#define soft_reset()        \
do                          \
{                           \
    wdt_enable(WDTO_15MS);  \
    for(;;)                 \
    {                       \
    }                       \
} while(0)

const unsigned long WAIT_TIME = 600000; 

uint8_t uiBufferRtx[] = {0x00,0x00};
uint8_t uiBufferRrx[] = {0x00,0x00};
MyMessage msgactiontemp(1,V_HVAC_SETPOINT_HEAT);
MyMessage msgactionlevel(2,V_LEVEL);
MyMessage msgcustom(10,V_CUSTOM);

MyMessage msgGw2Node(0,V_CUSTOM);
MyMessage msgNode2Gw(1,V_CUSTOM);

void before()
{

}

void setup()
{
  wait(20000);// wait while stove is booting
  Serial.println("Démarrage communication pellet stove ");
  SW_UART_Enable();

}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("NodePelletStove", "1.0");
}


void loop()
{
Read_Send_Stove_data();
wait(WAIT_TIME);
}

void receive(const MyMessage &message)
{
	if (message.sender== 0 && message.type==V_CUSTOM){
	uint8_t* data=(uint8_t*)message.getCustom();
	if (message.version_length>>3 == 2 ){
	switch(data[0])
{	
	case 1 : //on-off
	Serial.print("Bouton On/Off selectionné = ");
	repetition(0x80,0x58,0x5A,10,140);
	wait(25);
	Read_Send_Stove_data();
		break;
	case 2 : // consigne temp
	Serial.print("Message recu Temp Consigne = ");
	Serial.println(data[1]);
	bWriteStove(0xA0, 0x7D,data[1]);
	wait(25);
	Read_Send_Stove_data();
		break;
	case 3 : // consigne power
	Serial.print("Message recu Power Consigne = ");
	Serial.println(data[1]);
	bWriteStove(0xA0, 0x7F,data[1]);
	wait(25);
	Read_Send_Stove_data();
		break;
	case 4 : // refresh
	Serial.print("Message recu Mise à jour données = ");
	Read_Send_Stove_data();
		break;
	case 5 : // reset
	Serial.print("Reseting Node");
  soft_reset();
  while(true){}
		break;
		
	default : 
	Serial.println("Erreur de commande");
	break;
	
}
	
	}
	}
}

void Read_Send_Stove_data(){
	  uint8_t tempc=0;
	  uint8_t tempa=0;
	  uint8_t power=0;
	  uint8_t etat=0;
	  uint8_t erreur=0;
	  uint8_t message[4]={0};
  
  if(bReadStove(0x20,0x7D)){
  tempc = uiBufferRrx[1];
  message[0] = tempc;
  }
    Serial.print("Consigne Temp actuelle lue  = ");
  Serial.println(tempc);
  wait(25);
  if(bReadStove(0x00,0x01)){
  tempa= uiBufferRrx[1];
  message[1] = tempa;
  }
    Serial.print("Temp ambiante  = ");
  Serial.println(tempa/2.0);
   wait(25);
    if(bReadStove(0x20,0x7F)){
  power = uiBufferRrx[1];
  message[2]= power<<4;
  }
    Serial.print("Consigne Power actuelle lue  = ");
  Serial.println(power);
    wait(25);
   
     if(bReadStove(0x00,0x21)){
  etat= uiBufferRrx[1];
  message[2] |= etat;
  }
    Serial.print("Etat  = ");
  Serial.println(etat);
     wait(25);
  if(bReadStove(0x20,0xE5)){
  erreur= uiBufferRrx[1];
  message[3] = erreur;
  }
    Serial.print("Erreur  = ");
  Serial.println(erreur);
  
  send(msgNode2Gw.set(message,4));
}


bool bReadStove(uint8_t Type, uint8_t Addr){
  bool bDataValid = false;
  memset(uiBufferRrx, 0x00, 2);
  SW_UART_Transmit(Type);
  SW_UART_Transmit(Addr);
  
if (SW_UART_ReceiveBytes(uiBufferRrx, 2) == 2) {
    if(((uiBufferRrx[1] + Type + Addr) % 256) == uiBufferRrx[0]) {
      bDataValid = true;    
    }
    else {
      bDataValid = false;
      Serial.print("CS Error : Type :");
      Serial.print(Type);
      Serial.print(" Adrr : ");
      Serial.print(Addr);
      Serial.print(" Recei : ");
      Serial.print(uiBufferRrx[0]);
      Serial.print(" / ");
      Serial.println(uiBufferRrx[1]);      

    }
  }
  else {
    bDataValid = false;   
    Serial.println("TimeOut");    
  } 

return bDataValid;
}

bool bWriteStove(uint8_t Type, uint8_t Addr,uint8_t Data){
	bool bDataValid = false;
	memset(uiBufferRtx, 0x00, 2);
	
  SW_UART_Transmit(Type);
  SW_UART_Transmit(Addr);
  SW_UART_Transmit(Data);
  uint8_t cs = ((Type + Addr + Data) % 256);
  SW_UART_Transmit(cs);
  
if (SW_UART_ReceiveBytes(uiBufferRtx, 2) == 2) {
    if(uiBufferRtx[0] == cs && uiBufferRtx[1] == Data) {
      bDataValid = true;    
    }
    else {
      bDataValid = false;
      Serial.print("Error : Adrss :");
      Serial.print(uiBufferRtx[0]);
      Serial.print(" Data : ");
      Serial.print(uiBufferRtx[1]);
    }
  }
  else {
    bDataValid = false;   
    Serial.print("TimeOut, données transmises : 0x"); 
	Serial.print(Type,HEX);
	Serial.print(" 0x");
	Serial.print(Addr,HEX);
	Serial.print(" 0x");
	Serial.print(Data,HEX);
	Serial.print(" 0x");
	Serial.println(cs,HEX);
  }

return bDataValid;
}

void bWriteStoveWOReturn(uint8_t Type, uint8_t Addr,uint8_t Data){
  uint8_t cs = ((Type + Addr + Data) % 256);
  SW_UART_Transmit(Type);
  SW_UART_Transmit(Addr);
  SW_UART_Transmit(Data);
  SW_UART_Transmit(cs);
}


void repetition(uint8_t Type, uint8_t Addr,uint8_t Data,uint8_t nbrreptr, uint8_t interval_ms)
{

	 for(uint8_t i=1;i <= nbrreptr;i++){
		bWriteStoveWOReturn(Type,Addr,Data);
		wait(interval_ms);
	 }
	  
}
