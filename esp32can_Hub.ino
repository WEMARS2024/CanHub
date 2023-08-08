
#include "0_Core_Zero.h"


//extern char strCAN_RxGPS[200];
//extern char strCAN_TxIMU[200];
//extern unsigned int CR0_uiTxSequenceBuffer[11];
//extern unsigned int CR0_uiTxSequenceIndex = 0;


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
        LoadTxBuffer(100,0); //Request GPS data
        break;
      }
      case 'i':
      case 'I':
      {
        LoadTxBuffer(101,0); //request IMU data
        break;
      }
      case 'E':
      case 'e':
      {
        LoadTxBuffer(0,0);  //emergency stop
        break;
      }
    }
    

  }

  
}
