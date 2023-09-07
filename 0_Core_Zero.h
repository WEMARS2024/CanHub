/*
WE MARS
2023 08 03
CAN Hub
  
\Core 0 code


*/


#ifndef CORE_ZERO_H
#define CORE_ZERO_H 1


#include <Arduino.h>
#include <ESP32CAN.h>
#include <CAN_config.h>

CAN_device_t CAN_cfg;               // CAN Config
CAN_frame_t rx_frame;
CAN_frame_t tx_frame;
unsigned long previousMillis = 0;   // will store last time a CAN Message was send
const int interval = 1000;          // interval at which send CAN Messages (milliseconds)
const int rx_queue_size = 10;       // Receive Queue size

TaskHandle_t Core_Zero;


extern char strGPS[200];
extern char strIMU[200];



const int CR0_ciCANTimer =  10000;

char bTo_PI_Error_Code; bit 0 = GPS CAN RX PacketCorrupt, bit 1 = GPS Buffer overflow, ...

uint32_t CR0_u32Now;  //for timing testing
uint32_t CR0_u32Last;


unsigned long CR0_ulPreviousMicrosCore0;
unsigned long CR0_ulCurrentMicrosCore0;


unsigned int CR0_uiRx_EStop = 0;
unsigned int CR0_uiTxSequenceBuffer[11];
unsigned int CR0_uiTxSequenceIndex = 0;
unsigned int CR0_uiTxIndex = 0;
unsigned int CR0_uiTxPacketIndex = 0;
unsigned int CR0_uiTxPacketSize = 0;

unsigned int CR0_uiRxGPSPacketIndex = 0;
unsigned int CR0_uiGPSRxPacketIDExpected;
unsigned int CR0_uiIMURxPacketIDExpected;

unsigned int uiGPSBufferLoaded = 0;

int CR0_iTotalGPSRxPacketExpected;

char strCAN_RxGPS[200];
char strCAN_RxIMU[200];

void Core_ZeroCode( void * pvParameters );
void LoadRxData(unsigned int uiPacketIndex, bool btLastPacket);
unsigned int LoadTxBuffer(unsigned int uiId, unsigned int uiPacketCount,unsigned  int uiPacketIndx);

void Core_ZEROInit()
{
   xTaskCreatePinnedToCore(
                    Core_ZeroCode,   /* Task function. */
                    "Core_Zero",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Core_Zero,      /* Task handle to keep track of created task */
                    0);          /* pin task to core 0 */                  
  delay(500); 
};


void Core_ZeroCode( void * pvParameters )
{
  Serial.print("Core - ");
  Serial.print(xPortGetCoreID());
  Serial.println("   running ");


  //Core 0 Setup
  //-------------------------------------------------------------------------------------------
  CAN_cfg.speed = CAN_SPEED_125KBPS;
  CAN_cfg.tx_pin_id = GPIO_NUM_21;
  CAN_cfg.rx_pin_id = GPIO_NUM_22;
  CAN_cfg.rx_queue = xQueueCreate(rx_queue_size, sizeof(CAN_frame_t));
  // Init CAN Module
  ESP32Can.CANInit();
   
   
  //loop function for core 0
  //-------------------------------------------------------------------------------------------
  for(;;)
  {
       unsigned long currentMillis = millis();

      // Receive next CAN frame from queue
      if (xQueueReceive(CAN_cfg.rx_queue, &rx_frame, 3 * portTICK_PERIOD_MS) == pdTRUE)
       {
       
        if(rx_frame.MsgID == 0)
        {
          //emergency stop received
        
          
          CR0_uiRx_EStop = 0;
          CR0_uiTxSequenceIndex = 0;
          CR0_uiTxIndex = 0;
          CR0_uiTxSequenceBuffer[CR0_uiTxSequenceIndex] = 0;
          Serial.println("RX ZERO");
        
        }
        else if((rx_frame.MsgID >= 110) && (rx_frame.MsgID < 150))
        //receiving GPS data
        {
          if(CR0_uiGPSRxPacketIDExpected != rx_frame.MsgID) //lost data packet ignore rest of data
          {
            strcpy(strCAN_RxGPS,"G,InValid");
            bTo_PI_Error_Code |= 0x01;
          }
          else 
          {
            if((bTo_PI_Error_Code & 0x01)== 0)
            {
              if(CR0_uiGPSRxPacketIDExpected == 110) //First packet
              {
                CR0_iTotalGPSRxPacketExpected = rx_frame.data.u8[0]
                bTo_PI_Error_Code &= 0xFE;
                
              }
              else
              {
                CR0_uiRxGPSPacketIndex += 1;
                            

              }
              if(CR0_uiRxGPSPacketIndex > CR0_iTotalGPSRxPacketExpected)
               {
                  bTo_PI_Error_Code |= 0x01;
               }
               else if(CR0_uiRxGPSPacketIndex == CR0_iTotalGPSRxPacketExpected)
               {
                LoadRxData(CR0_uiRxGPSPacketIndex, 1);//last packet, set to pi
               }
               else
               {
                  LoadRxData(CR0_uiRxGPSPacketIndex, 0);
               }
                
              
            }
         }
        else if(rx_frame.MsgID >= 150)
        //receiving IMU data
        {
          

        }
            
            
            
            
            
      }
      
  }
}


void LoadRxData(unsigned int uiPacketIndex,bool btLastPacket)
{
  unsigned int uiIndexThroughString;

  if(rx_frame.FIR.B.DLC == 8)
  {
    for(uiIndexThroughString = 0;uiIndexThroughString < 8;uiIndexThroughString++)
    {
       strCAN_RxGPS[uiIndexThroughString + (uiPacketIndex * 8)] = rx_frame.data.u8[uiIndexThroughString];
    }
    if(btLastPacket)
    {
      //send string to PI
      if(uiGPSBufferLoaded)
      {
        bTo_PI_Error_Code |= 0x02;  //buffer data not transfered to PI
      }
      else
      {
        strcpy(strGPS,strCAN_RxGPS);
        strcpy(strCAN_RxGPS,"0");
        uiGPSBufferLoaded = 1;
      }
    }
  }
  
}


unsigned int LoadTxBuffer(unsigned int uiId, unsigned int uiPacketCount,unsigned  int uiPacketIndx)
{
 

  tx_frame.FIR.B.FF = CAN_frame_std;
  tx_frame.MsgID = uiId;
  tx_frame.FIR.B.DLC = 8;
  if(uiPacketCount == 0) //sending commands
  {
    tx_frame.FIR.B.DLC = 1;
    tx_frame.data.u8[0] = 0;
    return(0);
  }
  else  //sending CAN Data 
  {
    return(0);
  }
  

}



#endif
