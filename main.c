#include "LPC17xx.h"                    // Device header
#include "RTE_Components.h"             // Component selection
#include "Driver_USART.h"               // ARM::CMSIS Driver:USART:Custom
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#include "PIN_LPC17xx.h"                // Keil::Device:PIN
#include "RTE_Device.h"                 // Keil::Device:Startup
                  
//#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
                                
#include "Board_GLCD.h"                 // ::Board Support:Graphic LCD
#include "GLCD_Config.h"
#include <stdio.h>
#include <string.h>

#define ARM_USART_EVENT_RECEIVE_COMPLETE (1UL << 1)

#define IN_A       (1 << 16)  // GPIO P0.16
#define IN_B       (1 << 17)  // GPIO P0.17
#define EN_A       (1 << 19)  // GPIO P0.19 
#define EN_B       (1 << 18)  // GPIO P0.18 
#define PWM_PIN    (1 << 25)  // GPIO P3.25 (PWM Moteur)
#define PWM_Servo (1<<26)  // GPIO P3.26

// Configuration du PWM
#define PWM_FREQUENCY_Moteur     10000  	// 10 kHz pour le moteur


extern GLCD_FONT GLCD_Font_6x8;
extern GLCD_FONT GLCD_Font_16x24;

extern ARM_DRIVER_USART Driver_USART1;

char a;
char rx_buffer[20];
char C,Z,X,Y;


osThreadId ID_Receive;
osThreadId ID_Afficher_nunchuk;
osThreadId ID_moteur;

void Receive_UART(void const * argument);
void Afficher_nunchuk(void const * argument);
void moteur(void const * argument);

osMutexId ID_GLCD;
osMutexDef(GLCD0);


void InitGPIO(void);
void InitPWM(int);
void Avancer(char vitesse);
void Reculer(char vitesse);
void DelayMs(unsigned int ms);
void initTimer0 (int prescaler, int match);
void initTimer1 (int prescaler, int match);

void Init_UART(void);
void UART_Callback(uint32_t event);




osThreadDef (Receive_UART, osPriorityNormal, 1, 0);
osThreadDef (Afficher_nunchuk, osPriorityNormal, 1, 0);
osThreadDef (moteur, osPriorityNormal, 1, 0);

int main(void)
{
	osKernelInitialize ();
	
	GLCD_Initialize();
	GLCD_ClearScreen();
	GLCD_SetFont(&GLCD_Font_16x24);
	GLCD_SetBackgroundColor(GLCD_COLOR_WHITE);
	GLCD_SetForegroundColor(GLCD_COLOR_BLACK);

	Init_UART();
	
//	InitGPIO();
//  InitPWM(40);
//	LPC_GPIO3->FIODIR |=  PWM_Servo;       //P3.26 configuré en sortie
//  initTimer0( 999 , 499 ) ;       // PR et MR pour interuption tout les 20 ms
//  initTimer1( 999 , 34 ) ;       // PR et MR pour interuption tout les 20 ms
	

	
	
	ID_Receive = osThreadCreate(osThread(Receive_UART), NULL);
	ID_Afficher_nunchuk = osThreadCreate(osThread(Afficher_nunchuk), NULL);
	ID_moteur = osThreadCreate(osThread(Afficher_nunchuk), NULL);
	
	ID_GLCD=osMutexCreate(osMutex(GLCD0));
	
	osKernelStart();
	osDelay(osWaitForever) ;
	
	while (1)
  {
  }
}


void Init_UART(void){
	Driver_USART1.Initialize(UART_Callback);
	Driver_USART1.PowerControl(ARM_POWER_FULL);
	Driver_USART1.Control(	ARM_USART_MODE_ASYNCHRONOUS |
							ARM_USART_DATA_BITS_8		|
							ARM_USART_STOP_BITS_1		|
							ARM_USART_PARITY_NONE		|
							ARM_USART_FLOW_CONTROL_NONE,
							115200);
	Driver_USART1.Control(ARM_USART_CONTROL_TX,1);
	Driver_USART1.Control(ARM_USART_CONTROL_RX,1);
}
void TIMER0_IRQHandler(void) // Attention à bien nommer la fonction !
{
  LPC_TIM0->IR = (1<<0); //baisse le drapeau dû à MR0
  LPC_GPIO3->FIOPIN |= PWM_Servo;     //inverse l’état de P3.26
  LPC_TIM1->TCR = 1 ;                     // Lancement Timer 1 au debut de Timer 0
}
void TIMER1_IRQHandler(void) // Attention à bien nommer la fonction !
{
  LPC_TIM1->IR = (1<<0); //baisse le drapeau dû à MR0
  LPC_GPIO3->FIOPIN &=~ PWM_Servo;     //inverse l’état de P3.26
  LPC_TIM1->TCR = 0 ;                     // Arret Timer 1 
}

void initTimer0 (int prescaler, int match) 
{
  LPC_TIM0->PR = prescaler;                // Prescaler PR
  LPC_TIM0->MR0 = match;                   // valeur de MR
  LPC_TIM0->MCR = LPC_TIM0->MCR | (3<<0); // RAZ du compteur + interruption
  LPC_TIM0->TCR = 1 ;                     // Lancement Timer
  NVIC_SetPriority(TIMER0_IRQn,0);        // TIMER0 (IRQ1) : interruption de priorité 0
  NVIC_EnableIRQ(TIMER0_IRQn);            // active les interruptions TIMER0
}
void initTimer1 (int prescaler, int match) 
{
  LPC_TIM1->PR = prescaler;                // Prescaler PR
  LPC_TIM1->MR0 = match;                   // valeur de MR
  LPC_TIM1->MCR = LPC_TIM1->MCR | (3<<0); // RAZ du compteur + interruption
  
  NVIC_SetPriority(TIMER1_IRQn,0);        // TIMER0 (IRQ1) : interruption de priorité 0
  NVIC_EnableIRQ(TIMER1_IRQn);            // active les interruptions TIMER0
}






// Initialisation des GPIO
void InitGPIO(void) 
{
    LPC_GPIO0->FIODIR |= (IN_A | IN_B | EN_A | EN_B);  // Configurer comme sorties
    
//		LPC_GPIO3->FIODIR |= SERVO_PIN; // Configurer P3.26 comme sortie
}








// Initialisation du PWM -- Moteur
void InitPWM(int alpha)
{
		// Utilisation d la PWM1.2 
		// MR2 rapport cyclique alpha du moteur
		// ils varient de 0 à 99 (cas MR0 associé à 99)
		// La fréquence de la PWM vaut 10KHz soit 100 us
		// la fréquence de base est donnée par l'horloge interne de fréquence 25 MHz soir 40ns

		LPC_SC->PCONP = LPC_SC->PCONP | 0x00000040;   // enable PWM1

		// calcul des diviseurs
		// prescaler+1 = 1us/40ns = 25 cela donne une horloge de base de fréquence 10 KHz = 100 us
		// valeur + 1 = 100 cela fait 1ms 
		// la duree de comptage vaut 1 us

		LPC_PWM1->PR = 24;  // prescaler
		LPC_PWM1->MR0 = 99;    // MR0+1=100   la période de la PWM vaut 100µs

		LPC_PINCON->PINSEL7 = LPC_PINCON->PINSEL7| (3 << 18); // P3.25 est la sortie 1 PWM1   bit19 & bit18 

																	
		LPC_PWM1->MCR = LPC_PWM1->MCR | 0x00000002; // Compteur relancé quand MR0 repasse à 0
		LPC_PWM1->LER = LPC_PWM1->LER | 0x0000000F;  // ceci donne le droit de modifier dynamiquement la valeur du rapport cyclique
		// bit 0 = MR0    bit 1 MR1 bit2 MR2 bit3 = MR3
		LPC_PWM1->PCR = LPC_PWM1->PCR | 0x00000e00;  // autorise les sortie PWM1/2/3 bits 9, 10, 11


		LPC_PWM1->MR2 = alpha;							//Rapport cyclique alpha OU vitesse

		LPC_PWM1->TCR = 1;  /*validation de timer  et reset counter */
}





// Avancer avec une vitesse donnée
void Avancer(char vitesse) 
{
    LPC_GPIO0-> FIOPIN |= IN_A;					// IN_A à 1
    LPC_GPIO0-> FIOPIN &=~ IN_B;				// IN_B à 0
    LPC_GPIO0-> FIOPIN |= EN_A | EN_B;	// EN_A & EN_B à 1
    LPC_PWM1->MR2 = vitesse;						// Rapport cyclique alpha OU vitesse
}

// Reculer avec une vitesse donnée
void Reculer(char vitesse) 
{
    LPC_GPIO0-> FIOPIN |= IN_B;					// IN_B à 1 P0.17
		LPC_GPIO0-> FIOPIN &=~ IN_A; 				// IN_A à 0 P0.16
    LPC_GPIO0-> FIOPIN |= EN_A | EN_B;		// EN_A & EN_B à 1
    LPC_PWM1->MR2 = vitesse;						// Rapport cyclique alpha OU vitesse
}

void DelayMs(uint32_t ms) {
		uint32_t i ;
    for(i = 0; i < ms * 20000; i++) __NOP();
}



void UART_Callback(uint32_t event)
{		

		
		if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) 
		{
        osSignalSet(ID_Receive, 0x0002);		
    }
    	

}




void Receive_UART(void const *argument){
		uint8_t rx_buffer[9];
		char start;
		while(1)
		{
			//osDelay(100);
			
			osMutexWait(ID_GLCD,osWaitForever);

			


			Driver_USART1.Receive(&C, 1);			
			osSignalWait(0x0002, osWaitForever);
			Driver_USART1.Receive(&Z, 1);			
			osSignalWait(0x0002, osWaitForever);
			Driver_USART1.Receive(&X, 1);			
			osSignalWait(0x0002, osWaitForever);
			Driver_USART1.Receive(&Y, 1);			

			osSignalWait(0x0002, osWaitForever);
			
		
			
			osMutexRelease(ID_GLCD);
		}
}

void Afficher_nunchuk(void const *argument){
		
char a[50];
char b[50];
		while(1)
		{
		osMutexWait(ID_GLCD,osWaitForever);
		sprintf(a,"C=%d Z=%d ",C,Z);
		sprintf(b,"X=%3d Y=%3d",X,Y);
		GLCD_DrawString(10,10,a);
		GLCD_DrawString(10,50,b);
		
		osDelay(100);
		osMutexRelease(ID_GLCD);

		
		}
}

void moteur(void const *argument){
		osDelay(osWaitForever);
//char moteurA,moteurR;
//		while(1)
//		{
//		
//		if (Y>125)
//		{
//		moteurA=(Y-125);
//		Avancer(moteurA);
//		}
//		if (Y<125)
//		{
//		moteurR=(125-Y);
//		Reculer(moteurR);
//		}

//		
//		}
}

