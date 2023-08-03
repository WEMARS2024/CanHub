
#include "0_Core_Zero.h"


extern char strCAN_RxGPS[200];
extern char strCAN_TxIMU[200];

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

        break;
      }
    }
    

  }

  
}
