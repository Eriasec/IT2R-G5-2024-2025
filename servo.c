#include "LPC17xx.h"                    // Device header
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX


#define PWM_Servo (1<<26)  // GPIO P3.26

////Taches
void Recep_CommandeServo(void const * argument);
osThreadId ID_Tache_Commande_Servo;
osThreadDef(Recep_CommandeServo,osPriorityNormal, 1,0);

void PWMServo(void const * argument);
osThreadId ID_Tache_PWMServo;
osThreadDef(PWMServo,osPriorityNormal, 1,0);

////BAL
osMailQId ID_MessageServo;
osMailQDef(MessageServo,1,char);

////Prototypes fonctions
void TIMER0_IRQHandler(void); 			// Fonction d'interruption sur TIMER0
void TIMER1_IRQHandler(void); 			// Fonction d'interruption sur TIMER1
void initTimer0 (int prescaler, int match) ;
void initTimer1 (int prescaler, int match) ;

/////main
int main(void)
{
  osKernelInitialize ();
  
  //Initialisation des periphériques
  char direction=34; 						// entre 28-40 pour direction de tout à droite à tout à gauche // 34:milieu
  LPC_GPIO3->FIODIR |=  PWM_Servo;       	//P3.26 configuré en sortie
  initTimer0( 999 , 499 ) ;       			// PR et MR pour interuption tout les 20 ms
  initTimer1( 999 , direction ) ;       	// PR et MR pour interuption tout les x ms lié à la valeur de direction pour fixer le temps à l'état haut
  
  //Création des taches et BAL
  ID_Tache_Commande_Servo = osThreadCreate (osThread(Recep_CommandeServo),NULL);
  ID_Tache_PWMServo = osThreadCreate (osThread(PWMServo),NULL);
  ID_MessageServo = osMailCreate(osMailQ(MessageServo), NULL);
  
  osKernelStart();
  osDelay(osWaitForever);
}

//////////Definition des Taches///////////////////////
void Recep_CommandeServo(void const * argument)
{
	char *ptr;
	while(1)
	{
		///attente recep message Bluetooth
		
		///Conversion du message recu par Bluetooth en data traitable par le servo
		
		///Envoie message BAL pour modifier le signal PWMServo
		ptr = osMailAlloc(ID_MessageServo,osWaitForever);
		*ptr = message ; ////message reçu Bluetooth et à envoyer au Servo
		osMailPut(ID_MessageServo,ptr);		//Envoi
	}
}

void PWMServo(void const * argument)
{
	char *recep, valeur_recue_direction;
	osEvent EVretour;
	while(1)
	{
		EVretour = osMailGet(ID_MessageServo,osWaitForever); //Attente BAL
		recep = EVretour.value.p;							// On recupère le pointeur
		valeur_recue_direction = *recep ;					// On recupère la valeur pointé dans valeur_recue_direction
		osMailFree(ID_MessageServo,recep);					//Liberation de la memoire alloué au BAL
		
		initTimer1( 999 , valeur_recue_direction ) ;		//Modification du signal PWM avec un duty cycle de valeur_recue_direction/499*100
		
	}
}

//////Definition des fonctions///////////////////////

//////////Fonction d'interruption /////////////////////////////////////////////////////
void TIMER0_IRQHandler(void) 			// Fonction d'interruption sur TIMER0
{
  LPC_TIM0->IR = (1<<0); 				//baisse le drapeau dû à MR0
  LPC_GPIO3->FIOPIN |= PWM_Servo;   	//inverse l’état de P3.26
  LPC_TIM1->TCR = 1 ;                   // Lancement Timer 1 au debut de Timer 0
}
void TIMER1_IRQHandler(void) 			// Fonction d'interruption sur TIMER1
{
  LPC_TIM1->IR = (1<<0); 				//baisse le drapeau dû à MR0
  LPC_GPIO3->FIOPIN &=~ PWM_Servo;     	//inverse l’état de P3.26
  LPC_TIM1->TCR = 0 ;                   // Arret Timer 1 
}

//////////Fonction Timers /////////////////////////////////////////////////////
void initTimer0 (int prescaler, int match) 
{
  //Configuration du TIMER0
  LPC_TIM0->PR = prescaler;                // Prescaler PR
  LPC_TIM0->MR0 = match;                   // valeur de MR
  LPC_TIM0->MCR = LPC_TIM0->MCR | (3<<0); // RAZ du compteur + interruption
  LPC_TIM0->TCR = 1 ;                     // Lancement Timer
  
  //Configuration Interuption sur TIMER0
  NVIC_SetPriority(TIMER0_IRQn,0);        // TIMER0 (IRQ1) : interruption de priorité 0
  NVIC_EnableIRQ(TIMER0_IRQn);            // active les interruptions TIMER0
}
void initTimer1 (int prescaler, int match) 
{
  //Configuration du TIMER1
  LPC_TIM1->PR = prescaler;                // Prescaler PR
  LPC_TIM1->MR0 = match;                   // valeur de MR
  LPC_TIM1->MCR = LPC_TIM1->MCR | (3<<0); // RAZ du compteur + interruption
  
  //Configuration Interuption sur TIMER1
  NVIC_SetPriority(TIMER1_IRQn,0);        // TIMER0 (IRQ1) : interruption de priorité 0
  NVIC_EnableIRQ(TIMER1_IRQn);            // active les interruptions TIMER0
}
