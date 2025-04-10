/*---------------------------------------------------
* CAN 2 uniquement en TX 
* + r?ception CAN1 
* avec RTOS et utilisation des fonction CB
* pour test sur 1 carte -> relier CAN1 et CAN2
* 2017-04-02 - XM
---------------------------------------------------*/

//#define osObjectsPublic                     // define objects in main module
//#include "osObjects.h"                      // RTOS object definitions

//#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
//#include "stdio.h"
//#include "cmsis_os.h"


//osThreadId id_CANthreadT;

//extern   ARM_DRIVER_CAN         Driver_CAN2;


//// CAN2 utilis? pour ?mission
//void myCAN2_callback(uint32_t obj_idx, uint32_t event)
//{
//    switch (event)
//    {
//    case ARM_CAN_EVENT_SEND_COMPLETE:
//        /* 	Message was sent successfully by the obj_idx object.  */
//        osSignalSet(id_CANthreadT, 0x01);
//        break;
//    }
//}



//// CAN2 utilis? pour ?mission
//void InitCan2 (void) {
//	Driver_CAN2.Initialize(NULL,myCAN2_callback);
//	Driver_CAN2.PowerControl(ARM_POWER_FULL);
//	
//	Driver_CAN2.SetMode(ARM_CAN_MODE_INITIALIZATION);
//	Driver_CAN2.SetBitrate( ARM_CAN_BITRATE_NOMINAL,
//													125000,
//													ARM_CAN_BIT_PROP_SEG(5U)   |         // Set propagation segment to 5 time quanta
//                          ARM_CAN_BIT_PHASE_SEG1(1U) |         // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
//                          ARM_CAN_BIT_PHASE_SEG2(1U) |         // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
//                          ARM_CAN_BIT_SJW(1U));                // Resynchronization jump width is same as phase segment 2
//                          
//	// Mettre ici les filtres ID de r?ception sur objet 0
//	//....................................................
//		
//	Driver_CAN2.ObjectConfigure(2,ARM_CAN_OBJ_TX);				
//	
//	Driver_CAN2.SetMode(ARM_CAN_MODE_NORMAL);					// fin init
//	
//}


//// tache envoi toutes les secondes
//void CANthreadT(void const *argument)
//{
//	ARM_CAN_MSG_INFO                tx_msg_info;
//	uint8_t data_buf[8];
//	char i = 0;
//	while (1) {

//		ARM_CAN_MSG_INFO tx_msg_info;

//    tx_msg_info.id = ARM_CAN_STANDARD_ID(0x5F8); // ID 
//    tx_msg_info.rtr = 0;                         // Trame de Data 
//    
//    i++;
//    
//    data_buf[0] = i; 

//    Driver_CAN2.MessageSend(2, &tx_msg_info, data_buf, 1);

//		osSignalWait(0x01, osWaitForever);		// sommeil en attente fin emission
//		osDelay(100);
//	}		
//}


////osThreadDef(CANthreadR,osPriorityNormal, 1,0);
//osThreadDef(CANthreadT,osPriorityNormal, 1,0);

///*
// * main: initialize and start the system
// */
//int main (void) {
//  osKernelInitialize ();                    // initialize CMSIS-RTOS
//	
//	// Initialisation des 2 p?riph?riques CAN
//	InitCan2();

//  // create 'thread' functions that start executing,
//  // example: tid_name = osThreadCreate (osThread(name), NULL);
//	id_CANthreadT = osThreadCreate (osThread(CANthreadT), NULL);

//  osKernelStart ();                         // start thread execution 
//	osDelay(osWaitForever);
//}


