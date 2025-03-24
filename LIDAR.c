
#include "UART_LPC17xx.h"
#include "stdio.h"
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

// __________ Variables globales __________ \\


// __________ OS Threads __________ \\

void threadLidarRecep(void const * argument) {
	int k=0, i;
	unsigned char reception[50] = {0}, header[10], data[50];
	char state = 0, tab[10];
	
	while(1) {
		// _____ Attente du réveil _____
		osSignalWait(0x01, osWaitForever);
		switch(state) {
			case 0:
				Driver_USART1.Receive(reception, 7);
				if((reception[0] == LIDAR_START_HEADER) && (reception[1] == LIDAR_RESPONSE_HEADER)) {
					for(i=0; i<7; i++) {
						header[i] = reception[i];
					}
					GLCD_DrawChar(0,72,'r');
					state = 1;
				}
				break;
			case 1:
				Driver_USART1.Receive(reception, 5);
				for(i=0; i<5; i++) {
					data[i] = reception[i];
					sprintf(tab, "%x", data[i]);
					GLCD_DrawString(0,96,tab);
				}
				break;
//			case 2:
//				Driver_USART1.Receive(reception, 1);
//				header[k+2] = reception[0];
//				if(k >= 4) {
//					state = 3;
//				}
//				k++;
//				break;
		}
	}
}

void threadLidarEnvoi(void const * argument) {
//	char n = 0, tab[10];
	GLCD_DrawChar(0,24,'e');
	LIDAR_Scan();
	while(1) {
//		sprintf(tab, "%d", n);
//		GLCD_DrawString(0,48,tab);
//		LIDAR_Get_Info();
//		n++;
		osDelay(50);
	}
}

// __________ OS Thread Def __________ \\

osThreadDef(threadLidarRecep, osPriorityNormal, 1, 0);
osThreadDef(threadLidarEnvoi, osPriorityNormal, 1, 0);

// __________ Main __________ \\

int main(void) {
	// _____ Initialisation des threads _____
	osKernelInitialize();
	ID_ThreadLidarRecep = osThreadCreate(osThread(threadLidarRecep), NULL);
	ID_ThreadLidarEnvoi = osThreadCreate(osThread(threadLidarEnvoi), NULL);
	
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
	uint32_t mask;
	
	mask = ARM_USART_EVENT_RECEIVE_COMPLETE  |
         ARM_USART_EVENT_TRANSFER_COMPLETE |
         ARM_USART_EVENT_SEND_COMPLETE     |
         ARM_USART_EVENT_TX_COMPLETE       ;
	
	if(event & mask) {
		osSignalSet(ID_ThreadLidarRecep, 0x01);
	}
}
