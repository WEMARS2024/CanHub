
#include "0_Core_Zero.h"

#define MAX_TO_PI_BUFFER_SIZE 10

//extern char strCAN_RxGPS[200];
//extern char strCAN_TxIMU[200];
//extern unsigned int CR0_uiTxSequenceBuffer[11];
//extern unsigned int CR0_uiTxSequenceIndex = 0;
extern char bGPS_CAN_RX_PacketCorrupt;

char strToPIBuffer[MAX_TO_PI_BUFFER_SIZE];

unsigned int uiToPI_Index;
unsigned int uiToPI_Pointer;

void setup() {
  Serial.begin(115200);
  Serial.println("Rover Can Hub");
 
 
  Core_ZEROInit();


}

void loop()
{
  while (Serial.available() > 0) 
  {
    switch(Serial.read())
    {
      case '\n':
      case '\t':
      {
        break;
      }
      case 'g':
      case 'G':
      {
        LoadTxBuffer(100,0,0); //Request GPS data
        ESP32Can.CANWriteFrame(&tx_frame);
        CR0_uiGPSRxPacketIDExpected = 110;
        bGPS_CAN_RX_PacketCorrupt = 0;
        break;
      }
      case 'i':
      case 'I':
      {
        LoadTxBuffer(101,0,0); //request IMU data
        ESP32Can.CANWriteFrame(&tx_frame);
        CR0_uiIMURxPacketIDExpected = 150;
        break;
      }
      case 'E':
      case 'e':
      {
        LoadTxBuffer(0,0,0);  //emergency stop
        ESP32Can.CANWriteFrame(&tx_frame);
        break;
      }
    }
    

  }

  if(uiGPSBufferLoaded)
  {
    Serial.print(strGPS);
    uiGPSBufferLoaded = 0;
  }

  if(uiIMUBufferLoaded)
  {
    Serial.print(strIMU);
    uiIMUBufferLoaded = 0;
  }

  
}
