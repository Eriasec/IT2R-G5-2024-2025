#include "LPC17xx.h"                    // Device header
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO




// Définition des broches
#define IN_A       (1 << 16)  // GPIO P0.16
#define IN_B       (1 << 11)  // GPIO P0.11
#define EN_A       (1 << 19)  // GPIO P0.19 
#define EN_B       (1 << 18)  // GPIO P0.18 
#define PWM_PIN  (1 << 25)  // GPIO P3.25 (PWM Moteur)
#define SERVO_PIN  (1 << 26)  // GPIO P3.26 (PWM Servo) - Direction




// Configuration du PWM
#define PWM_FREQUENCY_Moteur     20000  // 20 kHz pour le moteur
#define PWM_FREQUENCY_Servo     		50  // 50 Hz pour le moteur
#define SERVO_MIN_PULSE   1000  				// 1000 µs pour 0°
#define SERVO_MAX_PULSE   2000 				 // 2000 µs pour 180°



// Prototypes des fonctions
void InitGPIO(void);
void InitPWM(void);
void SetServoAngle(unsigned int angle);
void Avancer(char vitesse);
void Reculer(char vitesse);
void DelayMs(unsigned int ms);

// Initialisation des GPIO
void InitGPIO(void) {
    LPC_GPIO0->FIODIR |= (IN_A | IN_B | EN_A | EN_B);  // Configurer comme sorties
    LPC_GPIO3->FIODIR |= PWM_PIN;  // Configurer P3.25 comme sortie pour PWM
		LPC_GPIO3->FIODIR |= SERVO_PIN; // Configurer P3.26 comme sortie
}

// Initialisation du PWM -- Moteur
void InitPWM(void) {
    LPC_SC->PCONP |= (1 << 6);  															// Activer PWM1
    LPC_PINCON->PINSEL3 |= (2 << 18) | (2 << 20); 						// Configurer P3.25 en PWM1.2 et P3.26 en PWM1.1
    
    LPC_PWM1->PR = 0; 																				// Pas de prescaler
    LPC_PWM1->MR0 = SystemCoreClock / PWM_FREQUENCY_Moteur; 	// Période du PWM - Match MR du PWM
    LPC_PWM1->MCR = (1 << 1); 																// Reset sur MR0 - RAZ du compteur
    LPC_PWM1->PCR = (1 << 9) | (1 << 10); 										// Activer PWM1.1 et PWM1.2
    LPC_PWM1->TCR = (1 << 0) | (1 << 3); 											// Activer le compteur (1<<0) et PWM
}

// Définir l'angle du servo de 0° à 180°
void SetServoAngle(unsigned int angle) {
    unsigned int pulse_width;
    unsigned int match_value;
    
    if (angle > 180) angle = 180;
    pulse_width = SERVO_MIN_PULSE + (angle * (SERVO_MAX_PULSE - SERVO_MIN_PULSE) / 180);
    match_value = (pulse_width * SystemCoreClock) / (PWM_FREQUENCY_Servo * 1000000);
    
    LPC_PWM1->MR1 = match_value;
    LPC_PWM1->LER = (1 << 1);
}

// Avancer avec une vitesse donnée
void Avancer(char vitesse) {
    LPC_GPIO0->FIOSET = IN_A;
    LPC_GPIO0->FIOCLR = IN_B;
    LPC_GPIO0->FIOSET = EN_A | EN_B;
    LPC_PWM1->MR2 = (vitesse * LPC_PWM1->MR0) / 100;
    LPC_PWM1->LER = (1 << 2);
}

// Reculer avec une vitesse donnée
void Reculer(char vitesse) {
    LPC_GPIO0->FIOCLR = IN_A;
    LPC_GPIO0->FIOSET = IN_B;
    LPC_GPIO0->FIOSET = EN_A | EN_B;
    LPC_PWM1->MR2 = (vitesse * LPC_PWM1->MR0) / 100;
    LPC_PWM1->LER = (1 << 2);
}

// Délai basé sur Timer0
void DelayMs(unsigned int ms) {
    LPC_TIM0->TCR = 0x02;  // Reset timer
    LPC_TIM0->PR = 0;      // Pas de prescaler
    LPC_TIM0->MR0 = (SystemCoreClock / 1000) * ms;
    LPC_TIM0->MCR = 0x04;  // Arrêt sur correspondance
    LPC_TIM0->TCR = 0x01;  // Démarrer le timer
    while (LPC_TIM0->TCR & 0x01);
}

// Fonction principale
int main(void) {
    char vitesse = 50;
    InitGPIO();
    InitPWM();

    while (1) {
        Avancer(vitesse);
        SetServoAngle(0);
        DelayMs(1000);

        Reculer(vitesse);
        SetServoAngle(90);
        DelayMs(1000);

        Avancer(vitesse);
        SetServoAngle(180);
        DelayMs(1000);
    }
}
