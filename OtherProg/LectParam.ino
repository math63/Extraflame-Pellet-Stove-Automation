#include <single_wire_UART.h>

//char* inData;
//uint8_t data[4];
//uint8_t index=0;
//char* pch;
uint16_t values[10];
uint8_t fieldIndex=0;
uint8_t type;
bool v;
uint8_t uiBufferRtx[] = {0x00,0x00};
uint8_t uiBufferRrx[] = {0x00,0x00};

void setup() {
  Serial.begin(9600);
  Serial.println("Attente reception commande : Lecture/ecriture(0,1),RAM/EPPROM (0,1), adresse (decimal), données (si ecriture)");
  Serial.println("ou repetion commande Attente 3 ,RAM/EPPROM (0,1), adresse (decimal), données, nbr repet, interval en ms (>50ms)");
  SW_UART_Enable();
}

void loop() {
  while(Serial.available() > 0) {
	  
	values[fieldIndex]=Serial.parseInt();
	fieldIndex++;
	
	if (Serial.read() == '\n') {
			uint8_t type;
			uint8_t addr;
			uint8_t val;
			uint8_t nb;
			uint16_t in;
		if (fieldIndex == 3)
		{
				if(values[0] ==0 && values[1] ==0)
			{
				type = 0x00;
				v=1;
			} else 	if(values[0] ==0 && values[1] ==1)
				{
				type = 0x20;
				v=1;
			} else
			{
				Serial.println("commande Invalide");
				v=0;
			}
			
			addr=values[2];
			if (v==1){
			Serial.print("Lecture de ");
			Serial.print( (values[1]) ? "EPPROM" : "RAM");
			Serial.print(" a l'adresse : ");
			Serial.print(values[2]);
			Serial.print(" (0x");
			Serial.print(values[2],HEX);
			Serial.println(")");
				if(bReadStove(type,addr)){
				Serial.print(" Réponse : ");
				Serial.print(uiBufferRrx[1]);
				Serial.print(" (0x");
				Serial.print(uiBufferRrx[1],HEX);
				Serial.println(")");
				} else {
					Serial.println(" Demande rejetée par le poele");
				}
			}


		} else if(fieldIndex == 4)
		{
			if(values[0] ==1 && values[1] ==0)
			{
				type = 0x80;
				v=1;
			} else 	if(values[0] ==1 && values[1] ==1)
				{
				type = 0xA0;
				v=1;
			} else
			{
				Serial.println("commande Invalide");
				v=0;
			}
			addr=values[2];
			val=values[3];
			
			if (v==1){
			Serial.print("Ecriture de ");
			Serial.print( (values[1]) ? "EPPROM" : "RAM");
			Serial.print(" a l'adresse : ");
			Serial.print(values[2]);
			Serial.print(" (0x");
			Serial.print(values[2],HEX);
			Serial.print(") avec la valeur : ");
			Serial.print(values[3]);
			Serial.print(" (0x");
			Serial.print(values[3],HEX);
			Serial.println(")");
			if(bWriteStove(type,addr,val)){
				Serial.println(" Commande validée ");
				} else {
					Serial.println(" Demande rejetée par le poele");
				}
			}
			
		}
		else if(fieldIndex == 6)
		{
			if(values[0] ==3 && values[1] ==0)
			{
				type = 0x80;
				v=1;
			} else 	if(values[0] ==3 && values[1] ==1)
				{
				type = 0xA0;
				v=1;
			} else
			{
				Serial.println("commande Invalide");
				v=0;
			}
			addr=values[2];
			val=values[3];
			nb=values[4];
			in=values[5];
			if (v==1){
			Serial.print("Repetition de ecriture ");
			Serial.print( (values[1]) ? "EPPROM" : "RAM");
			Serial.print(" a l'adresse : ");
			Serial.print(values[2]);
			Serial.print(" (0x");
			Serial.print(values[2],HEX);
			Serial.print(") avec la valeur : ");
			Serial.print(values[3]);
			Serial.print(" (0x");
			Serial.print(values[3],HEX);
			Serial.print(") ");
			Serial.print(values[4],DEC);
			Serial.print(" fois avec interval de  ");
			Serial.print(values[5]);
			Serial.println("ms");
			repetition(type,addr,val,nb,in);
			}
		}
		else
		{
		Serial.println("commande Invalide");
		v=0;
		}
			
		fieldIndex=0;
		
	}
}
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

void bWriteStoveworeturn(uint8_t Type, uint8_t Addr,uint8_t Data){
  uint8_t cs = ((Type + Addr + Data) % 256);
  SW_UART_Transmit(Type);
  SW_UART_Transmit(Addr);
  SW_UART_Transmit(Data);
  SW_UART_Transmit(cs);
}

void repetition(uint8_t Type, uint8_t Addr,uint8_t Data,uint8_t nbrreptr, uint16_t interval_ms){
const uint32_t enteringMS = millis();
	 for(uint8_t i=1;i <= nbrreptr;i++){
	while (millis() - enteringMS < interval_ms*(i-1)) 
	{}
		bWriteStoveworeturn(Type,Addr,Data);
	 }
	  
}