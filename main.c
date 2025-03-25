
#include "UART_LPC17xx.h"
#include "stdio.h"
#include "math.h"
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "GLCD_Fonts.h"                 // Keil.MCB1700::Board Support:Graphic LCD
#include "cmsis_os.h"

// __________ Dénifintion des octets utilisés __________ \\

#define LIDAR_START_HEADER 					0xA5
#define LIDAR_RESPONSE_HEADER				0x5A
#define LIDAR_REQUEST_RESET 				0x40
#define LIDAR_REQUEST_STOP 					0x25
#define LIDAR_REQUEST_SCAN					0x20
#define LIDAR_REQUEST_FORCE_SCAN		0x21
#define LIDAR_REQUEST_GET_INFO			0x50

extern GLCD_FONT GLCD_Font_16x24;

// __________ Prototypes Fonctions UART __________ \\

void Init_UART1(void);

// __________ Prototypes Fonctions LIDAR __________ \\

void LIDAR_Reset(void);
void LIDAR_Stop(void);
void LIDAR_Get_Info(void);		// Récupère les infos du lidar **Pour on ne lit pas les données renvoyées**
void LIDAR_Scan(void);
void LIDAR_Force_Scan(void);
void LIDAR_Attente_Header(void);

// __________ Prototypes Callback __________ \\

void callbackUSARTRecep(uint32_t event);

// __________ OS Thread ID __________ \\

osThreadId ID_ThreadLidarRecep;
osThreadId ID_ThreadLidarEnvoi;
osThreadId ID_ThreadLidarTraitement;

// __________ Variables globales __________ \\

typedef struct {
	uint8_t reception[200];
} maStructure;

// __________ OS MailBox __________ \\

osMailQId ID_BAL;
osMailQDef(BAL_Reception, 3, maStructure);



// __________ OS Threads __________ \\

void threadLidarRecep(void const * argument) {
	maStructure *t_ptr;
	
	uint8_t reception[200] = {0};
	char state = 0;
	
	ID_BAL = osMailCreate(osMailQ(BAL_Reception),NULL);
	
	// _____ Attente de la fin du send _____
	osSignalWait(0x02, osWaitForever);
	
	while(1) {
		switch(state) {
			case 0:
				Driver_USART1.Receive(reception, 7);		// _____ Reception du header _____
				osSignalWait(0x01, osWaitForever);			// _____ Attente de la fin du receive _____
				if((reception[0] == LIDAR_START_HEADER) && (reception[1] == LIDAR_RESPONSE_HEADER)) {		// _____ Vérification du header (0xA5, 0x5A)
					state = 1;
				}
				break;
				
			case 1:
				t_ptr = osMailAlloc(ID_BAL,osWaitForever);
				Driver_USART1.Receive(t_ptr->reception, 200);
				osSignalWait(0x01, osWaitForever);
				osMailPut(ID_BAL, t_ptr);
				break;
		}
	}
}

void threadLidarEnvoi(void const * argument) {
	GLCD_DrawChar(0,24,'e');
	LIDAR_Scan();
	while(1) {
		osDelay(osWaitForever);
	}
}
	
void threadLidarTraitement(void const * agument) {
	maStructure *r_ptr;
	osEvent evt;
	
	int i;
	uint16_t angle, distance, angleVeritable, distanceVeritable;
//	char tab[10];
	while(1) {
		evt = osMailGet(ID_BAL,osWaitForever);
		if(evt.status == osEventMail) {
			r_ptr = evt.value.p;
			for(i=0; i<200; i+=5) {
				if(r_ptr->reception[i] >= 0x20) {
					angle = 		(r_ptr->reception[i+1] & 0xFE) >> 1;		// _____ Octets 0 ?  6 de l'angle _____
					angle += 		(r_ptr->reception[i+2] & 0xFF) << 7;		// _____ Octets 7 ? 14 de l'angle _____
					distance = 	(r_ptr->reception[i+3] & 0xFF) << 0;		// _____ Octets 0 ?  7 de l'angle _____
					distance += (r_ptr->reception[i+4] & 0xFF) << 8;		// _____ Octets 8 ? 15 de l'angle _____
					angleVeritable = angle / 64;
					distanceVeritable = distance / 4;
					GLCD_DrawPixel(angleVeritable,distanceVeritable);
				}
			}
			osMailFree(ID_BAL, r_ptr);
		}
	}
}




// __________ OS Thread Def __________ \\

osThreadDef(threadLidarRecep, osPriorityAboveNormal, 1, 0);
osThreadDef(threadLidarEnvoi, osPriorityNormal, 1, 0);
osThreadDef(threadLidarTraitement, osPriorityNormal, 1, 0);






// __________ Main __________ \\

int main(void) {
	// _____ Initialisation des threads _____
	osKernelInitialize();
	ID_ThreadLidarRecep = osThreadCreate(osThread(threadLidarRecep), NULL);
	ID_ThreadLidarEnvoi = osThreadCreate(osThread(threadLidarEnvoi), NULL);
	ID_ThreadLidarTraitement = osThreadCreate(osThread(threadLidarTraitement), NULL);
	
	// _____ Initialisation de l'UART _____
	Init_UART1();
	
	// _____ Initialisation du GLCD _____
	GLCD_Initialize();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	GLCD_ClearScreen();
	GLCD_DrawChar(0,0,'i');
	
	// _____ Lancement RTOS _____
	osKernelStart();
	
	osDelay(osWaitForever);
	
	return 0;
}





// __________ Fonctions UART __________ \\

void Init_UART1(void) {
	Driver_USART1.Initialize(callbackUSARTRecep);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
													ARM_USART_DATA_BITS_8 |
													ARM_USART_STOP_BITS_1 |
													ARM_USART_PARITY_NONE |
													ARM_USART_FLOW_CONTROL_NONE,
													115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX, 1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX, 1);
}

// __________ Fonctions LIDAR __________ \\

void LIDAR_Get_Info(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_GET_INFO};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

void LIDAR_Scan(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_SCAN};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

void LIDAR_Reset(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_RESET};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

void LIDAR_Stop(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_STOP};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

void LIDAR_Force_Scan(void) {
	char tab[2] = {LIDAR_START_HEADER,LIDAR_REQUEST_FORCE_SCAN};
	
	while(Driver_USART1.GetStatus().tx_busy);
	Driver_USART1.Send(tab,2);
}

// __________ Fonctions Callback __________ \\

void callbackUSARTRecep(uint32_t event) {
	
	if(event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
		osSignalSet(ID_ThreadLidarRecep, 0x01);
	}
	if(event & ARM_USART_EVENT_SEND_COMPLETE) {
		osSignalSet(ID_ThreadLidarRecep, 0x02);
	}
}
