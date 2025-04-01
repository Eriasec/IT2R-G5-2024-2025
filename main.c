#include "LPC17xx.h"                    // Device header
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO
//#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX


// Définition des broches
#define IN_A       (1 << 16)  // GPIO P0.16
#define IN_B       (1 << 17)  // GPIO P0.17
#define EN_A       (1 << 19)  // GPIO P0.19 
#define EN_B       (1 << 18)  // GPIO P0.18 
#define PWM_PIN    (1 << 25)  // GPIO P3.25 (PWM Moteur)


// Configuration du PWM
#define PWM_FREQUENCY_Moteur     10000  	// 10 kHz pour le moteur

// Déclarations de tâches
//void tache1 (void const * argument) ;
// Prototypes des fonctions
void InitGPIO(void);
void InitPWM(int);
void Avancer(char vitesse);
void Reculer(char vitesse);
//void DelayMs(unsigned int ms);

//
//osThreadId ID_Moteur0 ;

//// Gestion de l'état du train en fonction des capteurs
//void tache1(void const * argument)
//{
//}



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

// Avancer avec un rapport cyclique donnée
void Avancer(char vitesse) 
{
    LPC_GPIO0-> FIOPIN |= IN_A;					// IN_A à 1
    LPC_GPIO0-> FIOPIN &=~ IN_B;				// IN_B à 0
    LPC_GPIO0-> FIOPIN |= EN_A | EN_B;	// EN_A & EN_B à 1
    LPC_PWM1->MR2 = vitesse;						// Rapport cyclique alpha OU vitesse
}


// Reculer avec un rapport cyclique donnée
void Reculer(char vitesse) 
{   
		LPC_GPIO0-> FIOPIN |= IN_B;					// IN_B à 1 P0.17 , Problème d'alim au niveau de IN_B
    LPC_GPIO0-> FIOPIN &=~ IN_A; 				// IN_A à 0 P0.16
    LPC_GPIO0-> FIOPIN |= EN_A | EN_B;		// EN_A & EN_B à 1
    LPC_PWM1->MR2 = vitesse;						// Rapport cyclique vitesse
}


//osThreadDef(tache1, osPriorityNormal, 1, 0);

// Fonction principale
int main(void) {

//		osKernelInitialize(); 
//    // Créer les tâches
//    ID_Moteur0 = osThreadCreate(osThread(tache1), NULL);
    InitGPIO();
    InitPWM(40);
while(1)
{
Avancer(40);
}
//    osKernelStart();        
//    osDelay(osWaitForever); 
  
}


