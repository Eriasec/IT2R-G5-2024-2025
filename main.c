
// R�solution ecran LCD 320x240

#include "UART_LPC17xx.h"
#include "stdio.h"
#include "math.h"
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"                // Keil.MCB1700::Board Support:Graphic LCD
#include "GLCD_Fonts.h"                 // Keil.MCB1700::Board Support:Graphic LCD
#include "cmsis_os.h"

// __________ D�nifintion des octets utilis�s __________ \\

#define LIDAR_START_HEADER 					0xA5
#define LIDAR_RESPONSE_HEADER				0x5A
#define LIDAR_REQUEST_RESET 				0x40
#define LIDAR_REQUEST_STOP 					0x25
#define LIDAR_REQUEST_SCAN					0x20
#define LIDAR_REQUEST_FORCE_SCAN		0x21
#define LIDAR_REQUEST_GET_INFO			0x50

#define TAILLE_BUFFER_RECEPTION			200
#define TAILLE_TRAME								5
#define N_TRAMES_BUFFER_RECEPTION		TAILLE_BUFFER_RECEPTION / TAILLE_TRAME
#define TAILLE_BUFFER_AFFICHAGE			N_TRAMES_BUFFER_RECEPTION * 4


extern GLCD_FONT GLCD_Font_16x24;

// __________ Prototypes Fonctions UART __________ 
void Init_UART1(void);

// __________ Prototypes Fonctions PWM __________ 
void InitPWM(int alpha);

// __________ Prototypes Fonctions LIDAR __________ 
void LIDAR_Reset(void);
void LIDAR_Stop(void);
void LIDAR_Get_Info(void);		// R�cup�re les infos du lidar **Pour on ne lit pas les donn�es renvoy�es**
void LIDAR_Scan(void);
void LIDAR_Force_Scan(void);

// __________ Prototypes Callback __________ \\

void callbackUSART(uint32_t event);

// __________ OS Thread ID __________ \\

osThreadId ID_ThreadLidarUART;
osThreadId ID_ThreadLidarTraitement;

// __________ Variables globales __________ \\

typedef struct {
	uint8_t reception[200];
} maStructure;

// __________ OS MailBox __________ \\

osMailQId ID_BAL;
osMailQDef(BAL_Reception, 3, maStructure);



// __________ OS Threads __________ \\

void threadLidarUART(void const * argument) {
	maStructure *t_ptr;
	
	uint8_t reception[200] = {0};
	char state = 0;
	
	ID_BAL = osMailCreate(osMailQ(BAL_Reception),NULL);
	
//	LIDAR_Reset();
//	osSignalWait(0x02, osWaitForever); // _____ Attente de la fin du send _____
//	osDelay(1000);
	
	LIDAR_Scan();
	osSignalWait(0x02, osWaitForever); // _____ Attente de la fin du send _____
	
	while(1) {
		switch(state) {
			case 0:
				Driver_USART1.Receive(reception, 7);		// _____ Reception du header _____
				osSignalWait(0x01, osWaitForever);			// _____ Attente de la fin du receive _____
				if((reception[0] == LIDAR_START_HEADER) && (reception[1] == LIDAR_RESPONSE_HEADER)) {		// _____ V�rification du header (0xA5, 0x5A)
					state = 1;
				}
				break;
				
			case 1:
				t_ptr = osMailAlloc(ID_BAL,osWaitForever);
				Driver_USART1.Receive(t_ptr->reception, TAILLE_BUFFER_RECEPTION);
				osSignalWait(0x01, osWaitForever);
				osMailPut(ID_BAL, t_ptr);
				break;
		}
	}
}





void threadLidarTraitement(void const * agument) {
	// Declaration des variables RTOS _____ //
	maStructure *r_ptr;				// _____ Pointeur vers la boite mail _____
	osEvent evt;							// _____ Event _____
	
	// _____ Declaration des variables _____ //
	int i, j;
	uint16_t angle, distance, angleVeritable, distanceVeritable;
	unsigned short x=0, y=0;
	unsigned short holdX[TAILLE_BUFFER_AFFICHAGE], holdY[TAILLE_BUFFER_AFFICHAGE];		// _____ Cr�ation d'un tableau � 2 lignes pour stoquer les anciennes valeurs des pixels pos�s (ligne 0 pour x et 1 pour y)_____
	char compteur = 0;
//	char tab[10];
	
	while(1) {
		evt = osMailGet(ID_BAL,osWaitForever);
		if(evt.status == osEventMail) {						// _____ Attente de la reception du mail _____
			r_ptr = evt.value.p;
			j = 0;
			
			for(i=0; i < TAILLE_BUFFER_RECEPTION; i += TAILLE_TRAME) {									// _____ On explore les trames receptionn�es _____
				if(r_ptr->reception[i] == 0x3E) {			// _____ Si la qualit� du laser n'est pas suffisante on ignore la trame _____
					angle = 		(r_ptr->reception[i+1] & 0xFE	) >> 1;		// _____ Octets 0 a  6 de l'angle _____
					angle |= 		(r_ptr->reception[i+2]				) << 7;		// _____ Octets 7 a 14 de l'angle _____
					distance = 	(r_ptr->reception[i+3]				) << 0;		// _____ Octets 0 a  7 de l'angle _____
					distance |= (r_ptr->reception[i+4]				) << 8;		// _____ Octets 8 a 15 de l'angle _____
					angleVeritable = angle >> 6;												// _____ Division par 64 pour obtenir le vrai angle _____
					distanceVeritable = (distance >> 2) - 100;					// _____ Division par 4 pour obtenir la vraie distance et - 100 car a moins de 10cm la qualite est trop basse _____
					distanceVeritable = distanceVeritable >> 2;					// _____ Mise a l'echelle _____
					x = (cos(((double) angleVeritable)*3.1415/180) * distanceVeritable) + 160;				// _____ calcul de la composante en x (+160 pour afficher au milieu) _____
					y = (sin(((double) angleVeritable)*3.1415/180) * distanceVeritable) + 120;				// _____ calcul de la composante en y (+120 pour afficher au milieu) _____
					// _____ On emp�che les valeurs de d�passer la taille de l'�cran _____
					if(y > 240) {
						y = 240;
					} if(x > 320) {
						x = 320;
					}
					
					GLCD_SetForegroundColor(GLCD_COLOR_WHITE);
//					GLCD_DrawPixel(	holdX	[j + (((compteur+3)&0x03) * N_TRAMES_BUFFER_RECEPTION)],
//													holdY [j + (((compteur+3)&0x03) * N_TRAMES_BUFFER_RECEPTION)]);
					GLCD_DrawPixel(	holdX	[j],
													holdY [j]);
					
					GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
					GLCD_DrawPixel(x,y);			// _____ On dessine les points pour repr�senter les objets alentours _____
					holdX[j + (compteur * N_TRAMES_BUFFER_RECEPTION)] = x;
					holdY[j + (compteur * N_TRAMES_BUFFER_RECEPTION)] = y;
					j += 1;
				}
			}
			
			compteur += 1;			// ----- ?? "compteur++" ne fonctionne pas ?? -----
			compteur &= 0x03;
			
			osMailFree(ID_BAL, r_ptr);			// _____ On lib�re la case de la boite mail qu'on vient de vider _____
		}
	}
}




// __________ OS Thread Def __________ \\

osThreadDef(threadLidarUART, osPriorityAboveNormal, 1, 0);		
osThreadDef(threadLidarTraitement, osPriorityNormal, 1, 0);			






// __________ Main __________ \\

int main(void) {
	int i,j;
	char tab[10];
	// _____ Initialisation des threads _____
	osKernelInitialize();
	ID_ThreadLidarUART = osThreadCreate(osThread(threadLidarUART), NULL);
	ID_ThreadLidarTraitement = osThreadCreate(osThread(threadLidarTraitement), NULL);
	
	// _____ Initialisation de l'UART _____
	Init_UART1();
	InitPWM(600);
	
	// _____ Initialisation du GLCD _____
	GLCD_Initialize();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);
	GLCD_ClearScreen();
	sprintf(tab, "%4d %4d", N_TRAMES_BUFFER_RECEPTION, TAILLE_BUFFER_AFFICHAGE);
	GLCD_DrawString(0, 0, tab);
	for(i=0; i<3; i++) {
		for(j=0; j<3; j++) {
			GLCD_DrawPixel(i+159, j+119);
		}
	}
	
	// _____ Lancement RTOS _____
	osKernelStart();
	
	osDelay(osWaitForever);
	
	return 0;
}





// __________ Fonctions UART __________ \\

void Init_UART1(void) {
	Driver_USART1.Initialize(callbackUSART);								// Initialisation avec callback
	Driver_USART1.PowerControl(ARM_POWER_FULL);							// Allimentation par la carte
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |		// Mode asynchrone
													ARM_USART_DATA_BITS_8 |					// 8 bits de donn�e
													ARM_USART_STOP_BITS_1 |					// 1 bit de stop
													ARM_USART_PARITY_NONE |					// Pas de parit�
													ARM_USART_FLOW_CONTROL_NONE,		// Pas de contr�le de flux
													115200);												// 115200 bauds
	Driver_USART1.Control(ARM_USART_CONTROL_TX, 1);					// Activation TX
	Driver_USART1.Control(ARM_USART_CONTROL_RX, 1);					// Activation RX
}

// __________ Fonctions PWM __________ \\

void InitPWM(int alpha) {
		// Utilisation d la PWM1.2 
		// MR2 rapport cyclique alpha du moteur
		// ils varient de 0 � 99 (cas MR0 associ� � 99)
		// La fr�quence de la PWM vaut 10KHz soit 100 us
		// la fr�quence de base est donn�e par l'horloge interne de fr�quence 25 MHz soir 40ns

		LPC_SC->PCONP = LPC_SC->PCONP | 0x00000040;   // enable PWM1

		// calcul des diviseurs
		// prescaler+1 = 1us/40ns = 25 cela donne une horloge de base de fr�quence 10 KHz = 100 us
		// valeur + 1 = 100 cela fait 1ms 
		// la duree de comptage vaut 1 us

		LPC_PWM1->PR = 0;  // prescaler
		LPC_PWM1->MR0 = 999;    // MR0+1=100   la p�riode de la PWM vaut 100�s

		LPC_PINCON->PINSEL7 = LPC_PINCON->PINSEL7| (3 << 18); // P3.25 est la sortie 1 PWM1   bit19 & bit18 

																	
		LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Compteur relanc� quand MR0 repasse � 0
		LPC_PWM1->LER = LPC_PWM1->LER | 0x0000000F;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
		// bit 0 = MR0    bit 1 MR1 bit2 MR2 bit3 = MR3
		LPC_PWM1->PCR = LPC_PWM1->PCR | 0x00000e00;  // autorise les sortie PWM1/2/3 bits 9, 10, 11


		LPC_PWM1->MR2 = alpha;							//Rapport cyclique alpha OU vitesse

		LPC_PWM1->TCR = 1;  /*validation de timer  et reset counter */
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

void callbackUSART(uint32_t event) {
	if(event & ARM_USART_EVENT_RECEIVE_COMPLETE) {		// Check si r�veil sur fin de reception
		osSignalSet(ID_ThreadLidarUART, 0x01);				// R�veil de la tache UART sur �venement 0x01
	}
	if(event & ARM_USART_EVENT_SEND_COMPLETE) {				// Check si r�veil sur fin d'envoi
		osSignalSet(ID_ThreadLidarUART, 0x02);				// R�veil de la tache UART sur �venement 0x01
	}
}
