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
#define STOVE_DEBUG

// Enable and select radio type attached
#define MY_RADIO_RFM95

#define MY_NODE_ID 1

#include <MySensors.h>
#include <single_wire_UART.h>
#include <avr/wdt.h>
#include <TimeLib.h> 

#define soft_reset()        \
do                          \
{                           \
    wdt_enable(WDTO_15MS);  \
    for(;;)                 \
    {                       \
    }                       \
} while(0)

// Wait time between updates (in milliseconds)
static const uint64_t WAIT_TIME = 600000;
// Time between sedning and receiving data from stove
static const uint64_t PEL_TIME= 25;


uint8_t uiBufferRtx[] = {0x00,0x00};
uint8_t uiBufferRrx[] = {0x00,0x00};

static MyMessage MsgNo2Gw_std(1,V_CUSTOM);
static MyMessage MsgNo2Gw_prog(2,V_CUSTOM);
static MyMessage MsgNo2Gw_para(3,V_CUSTOM);
static MyMessage MsgNo2Gw_deb(4,V_CUSTOM);

void before()
{

}

void setup()
{
  wait(30000);// wait while stove is booting
#ifdef STOVE_DEBUG
  Serial.println("Démarrage communication pellet stove ");
#endif

  SW_UART_Enable();

}

void presentation()
{
	// Send the sketch version information to the gateway and Controller
	sendSketchInfo("NodePelletStove", "2.2");
}


void loop()
{
  
Read_Send_Stove_Std_data();

wait(WAIT_TIME);
}

void receive(const MyMessage &message)
{
  
	if (message.sender== 0 && message.type==V_CUSTOM){
	static uint8_t* data=(uint8_t*)message.getCustom();
	if (message.version_length>>3 == 2 ){
	switch(data[0])
{	
	case 1 : //on-off
	#ifdef STOVE_DEBUG
	Serial.print("Bouton On/Off selectionné ");
	#endif
	repetition(0x80,0x58,0x5A,10,140);
	wait(PEL_TIME);
	Read_Send_Stove_Std_data();
		break;
	case 2 : // consigne temp
	#ifdef STOVE_DEBUG
	Serial.print("Message recu Temp Consigne = ");
	Serial.println(data[1]);
	#endif
	bWriteStove(0xA0, 0x7D,data[1]);
	wait(PEL_TIME);
	Read_Send_Stove_Std_data();
		break;
	case 3 : // consigne power
	#ifdef STOVE_DEBUG
	Serial.print("Message recu Power Consigne = ");
	Serial.println(data[1]);
	#endif
	bWriteStove(0xA0, 0x7F,data[1]);
	wait(PEL_TIME);
	Read_Send_Stove_Std_data();
		break;
  	case 4 : // refresh
	#ifdef STOVE_DEBUG
	Serial.print("Demande Mise à jour données ");
	#endif
	Read_Send_Stove_Std_data();
		break;
	case 5 : // reset
	#ifdef STOVE_DEBUG
	Serial.print("Reseting Node");
	#endif
  soft_reset();
  while(true){}
		break;
	case 6 : // Activation/Desactivation Programmation quotidienne
		#ifdef STOVE_DEBUG
	Serial.print("Changement état Programmation quotidienne : ");
	Serial.println(data[1]);
	#endif
	bWriteStove(0xA0, 0x4C, data[1]);
	wait(PEL_TIME);
	Read_Send_Stove_Std_data();
    break;  
   case 7 : // Activation/Desactivation fonction stand-by
    #ifdef STOVE_DEBUG
  Serial.print("Changement état fonction stand-by : ");
  Serial.println(data[1]);
  #endif
  bWriteStove(0xA0, 0x45, data[1]);
  wait(PEL_TIME);
  Read_Send_Stove_Std_data();
    break;  
    
   case 8 : // Mise à jour heure
  requestTime();
    break;

   case 9 : // Exploration RAM par lignes
   {
  #ifdef STOVE_DEBUG
  Serial.print("Exploration RAM de la lignes  : ");
  Serial.println(data[1]);
  #endif
  uint8_t debug[16]={0};
  
   for(uint8_t j=0;j <= 15;j++){
    bReadStove(0x00,data[1]*16+j);
    debug[j] = uiBufferRrx[1];
    wait(PEL_TIME);
   } 
   send(MsgNo2Gw_deb.set(debug,16));
   }
    break;

   case 10 : // Exploration ROM par lignes
    {
  #ifdef STOVE_DEBUG
  Serial.print("Exploration ROM de la lignes  : ");
  Serial.println(data[1]);
  #endif
  uint8_t debug[16]={0};
  
   for(uint8_t j=0;j <= 15;j++){
    bReadStove(0x20,data[1]*16+j);
    debug[j] = uiBufferRrx[1];
    wait(PEL_TIME);
   } 
   send(MsgNo2Gw_deb.set(debug,16));
    }
    break;
    
	default : 
	#ifdef STOVE_DEBUG
	Serial.println("Erreur de commande");
	#endif
	break;
	
}
	
	}
	}
}

bool Read_Send_Stove_Std_data(){
	  static uint8_t tempc=0;
	  static uint8_t tempa=0;
	  static uint8_t power=0;
	  static uint8_t etat=0;
	  static uint8_t prog=0;
    static uint8_t stand=0;
    static uint8_t temp_fum=0;
    static uint8_t temp_res=0;
    static uint8_t temp_inair=0;
	  static uint8_t erreur=0;
    static uint8_t cad_vis=0;
    static uint8_t vit_fum=0;
    static uint8_t dur_phase=0;
    static uint8_t flux=0;
    static uint8_t power_real=0;
	  static uint8_t message[13]={0};

    bool ReadOk=1;
    
   if(bReadStove(0x20,0x7D)){
  tempc = uiBufferRrx[1];
  message[0] = tempc;
    #ifdef STOVE_DEBUG
    Serial.print("Consigne Temp actuelle lue  = ");
    Serial.println(tempc);
    #endif
  }else{
    ReadOk=0;
  }
  
  wait(PEL_TIME);
  
  if(bReadStove(0x00,0x01)){
  tempa= uiBufferRrx[1];
  message[1] = tempa;
    #ifdef STOVE_DEBUG
    Serial.print("Temp ambiante  = ");
    Serial.println(tempa/2.0);
    #endif
  }else{
    ReadOk=0;
  }

   wait(PEL_TIME);
   
  if(bReadStove(0x20,0x7F)){
  power = uiBufferRrx[1];
  message[2]= power<<4;
    #ifdef STOVE_DEBUG
    Serial.print("Consigne Power actuelle lue  = ");
    Serial.println(power);
    #endif
  }else{
    ReadOk=0;
  }
  
    wait(PEL_TIME);
   
  if(bReadStove(0x00,0x21)){
  etat= uiBufferRrx[1];
  message[2] |= etat;
    #ifdef STOVE_DEBUG
    Serial.print("Etat  = ");
    Serial.println(etat);
    #endif
  }else{
    ReadOk=0;
  }

    wait(PEL_TIME);
    
  if(bReadStove(0x20,0x4C)){
  prog = uiBufferRrx[1];
  message[3] = prog <<7;
      #ifdef STOVE_DEBUG
      Serial.print("Etat Programmation = ");
      Serial.println(prog);
      #endif
  }else{
    ReadOk=0;
  }

  wait(PEL_TIME);
  
    if(bReadStove(0x20,0x45)){
  stand = uiBufferRrx[1];
  message[3] |= stand <<6;
      #ifdef STOVE_DEBUG
      Serial.print("Etat stand by = ");
      Serial.println(stand);
      #endif
  }else{
    ReadOk=0;
  }

     wait(PEL_TIME);
     
  if(bReadStove(0x20,0xE5)){
  erreur= uiBufferRrx[1];
  //erreur = (erreur > 0) ? 2 + int(log(erreur) / log(2)) : 1;
  message[4] = erreur;
      #ifdef STOVE_DEBUG
      Serial.print("Erreur  = ");
      Serial.println(erreur);
      #endif
  }else{
    ReadOk=0;
  }

    wait(PEL_TIME);
  
    if(bReadStove(0x00,0x5A)){
  temp_fum = uiBufferRrx[1];
  message[5] = temp_fum ;
      #ifdef STOVE_DEBUG
      Serial.print("temp fumées = ");
      Serial.println(temp_fum);
      #endif
  }else{
    ReadOk=0;
  }

      wait(PEL_TIME);
  
    if(bReadStove(0x00,0x03)){
  temp_res = uiBufferRrx[1];
  message[6] = temp_res ;
      #ifdef STOVE_DEBUG
      Serial.print("temp reservoire = ");
      Serial.println(temp_res);
      #endif
  }else{
    ReadOk=0;
  }

      wait(PEL_TIME);
  
    if(bReadStove(0x00,0x81)){
  temp_inair = uiBufferRrx[1];
  message[7] = temp_inair ;
      #ifdef STOVE_DEBUG
      Serial.print("temp entrée air = ");
      Serial.println(temp_inair/2.0-15.0);
      #endif
  }else{
    ReadOk=0;
  }

        wait(PEL_TIME);
  
    if(bReadStove(0x00,0x0D)){
  cad_vis = uiBufferRrx[1];
  message[8] = cad_vis ;
      #ifdef STOVE_DEBUG
      Serial.print("Cadence vis = ");
      Serial.println(cad_vis/40.0);
      #endif
  }else{
    ReadOk=0;
  }

        wait(PEL_TIME);
  
    if(bReadStove(0x00,0x37)){
  vit_fum = uiBufferRrx[1];
  message[9] = vit_fum ;
      #ifdef STOVE_DEBUG
      Serial.print("Vitesse fumées = ");
      Serial.println(vit_fum*10+250);
      #endif
  }else{
    ReadOk=0;
  }

          wait(PEL_TIME);
  
    if(bReadStove(0x00,0x32)){
  dur_phase = uiBufferRrx[1];
  message[10] = dur_phase ;
      #ifdef STOVE_DEBUG
      Serial.print("Durée restante dans la phase (min) = ");
      Serial.println(dur_phase);
      #endif
  }else{
    ReadOk=0;
  }

  
          wait(PEL_TIME);
  
    if(bReadStove(0x00,0x7E)){
  flux = uiBufferRrx[1];
  message[11] = flux ;
      #ifdef STOVE_DEBUG
      Serial.print("Flux  = ");
      Serial.println(flux);
      #endif
  }else{
    ReadOk=0;
  }

            wait(PEL_TIME);
  
    if(bReadStove(0x00,0x34)){
  power_real = uiBufferRrx[1];
  message[12] = power_real ;
      #ifdef STOVE_DEBUG
      Serial.print("Puissance réelle  = ");
      Serial.println(power_real);
      #endif
  }else{
    ReadOk=0;
  }
 
if(!send(MsgNo2Gw_std.set(message,13))){;
ReadOk=0;}

return ReadOk;
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
#ifdef STOVE_DEBUG
      Serial.print("CS Error : Type :");
      Serial.print(Type);
      Serial.print(" Adrr : ");
      Serial.print(Addr);
      Serial.print(" Recei : ");
      Serial.print(uiBufferRrx[0]);
      Serial.print(" / ");
      Serial.println(uiBufferRrx[1]);      
#endif
    }
  }
  else {
    bDataValid = false;
#ifdef STOVE_DEBUG
    Serial.println("TimeOut");
#endif    
  } 

return bDataValid;
}

bool bWriteStove(uint8_t Type, uint8_t Addr,uint8_t Data){
	bool bDataValid = false;
	memset(uiBufferRtx, 0x00, 2);
 
	uint8_t cs = ((Type + Addr + Data) % 256);
  
  SW_UART_Transmit(Type);
  SW_UART_Transmit(Addr);
  SW_UART_Transmit(Data);
  SW_UART_Transmit(cs);
  
if (SW_UART_ReceiveBytes(uiBufferRtx, 2) == 2) {
    if(uiBufferRtx[0] == cs && uiBufferRtx[1] == Data) {
      bDataValid = true;    
    }
    else {
      bDataValid = false;
#ifdef STOVE_DEBUG
      Serial.print("Error : Adrss :");
      Serial.print(uiBufferRtx[0]);
      Serial.print(" Data : ");
      Serial.print(uiBufferRtx[1]);
#endif
    }
  }
  else {

    bDataValid = false; 
#ifdef STOVE_DEBUG  
  Serial.print("TimeOut, données transmises : 0x"); 
	Serial.print(Type,HEX);
	Serial.print(" 0x");
	Serial.print(Addr,HEX);
	Serial.print(" 0x");
	Serial.print(Data,HEX);
	Serial.print(" 0x");
	Serial.println(cs,HEX);
#endif
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

void receiveTime(uint32_t controllerTime) {
  setTime(controllerTime);
  #ifdef STOVE_DEBUG
  Serial.print("Time value received: ");
  Serial.println(controllerTime);
  #endif
  bWriteStove(0xA0, 0xFD,year()%100);
  Serial.print("Année: ");
  Serial.println(year());
  wait(PEL_TIME);
  bWriteStove(0xA0, 0xFC,month());
  Serial.print("Mois: ");
  Serial.println(month());
  wait(PEL_TIME);
  bWriteStove(0xA0, 0xFB,day());
  Serial.print("Jour: ");
  Serial.println(day());
  wait(PEL_TIME);
  bWriteStove(0xA0, 0xFB,hour());
  Serial.print("Heure: ");
  Serial.println(hour());
  wait(PEL_TIME);
    bWriteStove(0xA0, 0xF9,minute());
      Serial.print("Minute: ");
  Serial.println(minute());
  wait(PEL_TIME);
  bWriteStove(0xA0, 0xF8,weekday());
  Serial.print("jour sem: ");
  Serial.println(weekday());
  wait(PEL_TIME);
  bWriteStove(0xA0, 0xF7,second());
    Serial.print("seconde: ");
  Serial.println(second());
}
