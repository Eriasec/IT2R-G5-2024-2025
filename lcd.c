#include "LPC17xx.h"                    // Device header
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "stdio.h"
#include "string.h"
#include "cmsis_os.h"
#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions

//axe x = 14.5 cm
//axe y = 10.5 cm
//

extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

osThreadId id_CANthreadR;    //MUTEX

extern   ARM_DRIVER_CAN         Driver_CAN1;

ARM_CAN_MSG_INFO   rx_msg_info;
uint8_t data_buf[8];

double lat_minsec;
double lon_minsec;



void conversionDMS(float lat, float lon){
  int lat_deg = (int)(lat/100);
  double lat_minsec = lat-(lat_deg*100);
  int lon_deg = (int)(lon/100);
  double lon_minsec = lon-(lon_deg*100);
  
}

void myCAN1_callback(uint32_t obj_idx, uint32_t event)
{
    if (event & ARM_CAN_EVENT_RECEIVE)
    {
        /*  Message was received successfully by the obj_idx object. */
      osSignalSet(id_CANthreadR,0x01);

    }
}



void InitCan1 (void) {
	Driver_CAN1.Initialize(NULL,myCAN1_callback);
	Driver_CAN1.PowerControl(ARM_POWER_FULL);
	
	Driver_CAN1.SetMode(ARM_CAN_MODE_INITIALIZATION);
	Driver_CAN1.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
													125000,
													ARM_CAN_BIT_PROP_SEG(5U)   |         // Set propagation segment to 5 time quanta
                          ARM_CAN_BIT_PHASE_SEG1(1U) |         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                          ARM_CAN_BIT_PHASE_SEG2(1U) |         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                          ARM_CAN_BIT_SJW(1U));                // Resynchronization jump width is same as phase segment 2
                          
	// Mettre ici les filtres ID de r?ception sur objet 0
	Driver_CAN1.ObjectSetFilter(0,ARM_CAN_FILTER_ID_EXACT_ADD,ARM_CAN_STANDARD_ID(0x5f8),0);
		
	Driver_CAN1.ObjectConfigure(0,ARM_CAN_OBJ_RX);				// Objet 0 du CAN1 pour r?ception
	
	Driver_CAN1.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
}


void CANthreadR(void const *argument)
{
	
	
	char texte[40];     //Affichage Latitude
  char texte1[40];    //Affichage Longitude
	int identifiant;
	float flatitude;
  float latitude;
  float longitude;
  
	
	
	while(1)
	{	
    
		osSignalWait(0x01, osWaitForever);		// sommeil, en attente de reception
		// Code pour reception trame + affichage Id et Data sur LCD
    Driver_CAN1.MessageRead(0,&rx_msg_info,data_buf,8);
 
		memcpy(&latitude, &data_buf[0],4);
    memcpy(&longitude, &data_buf[4],4);
		identifiant=rx_msg_info.id;
		
		sprintf(texte,"%03X,  %f", identifiant,latitude);
    sprintf(texte1,"%f",longitude);
		GLCD_DrawString(10,10,(unsigned char*)texte);
    GLCD_DrawString(10,80,(unsigned char*)texte1);
	}
  
}


osThreadDef(CANthreadR,osPriorityNormal, 1,0);


int main (void) {
  osKernelInitialize ();                    

  
	GLCD_Initialize();
	GLCD_ClearScreen();
  GLCD_SetFont(&GLCD_Font_16x24);
	
	
	
	InitCan1();
	  
	id_CANthreadR = osThreadCreate (osThread(CANthreadR), NULL);


  osKernelStart ();                         
	osDelay(osWaitForever);
}
