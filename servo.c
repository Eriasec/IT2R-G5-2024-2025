#include "LPC17xx.h"                    // Device header
#include "GPIO_LPC17xx.h"               // Keil::Device:GPIO

#define PWM_Servo (1<<26)  // GPIO P3.26

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

int main(void)
{
  char direction=34; // entre 28-40 pour direction de tout à droite à tout à gauche // 34:milieu
  LPC_GPIO3->FIODIR |=  PWM_Servo;       //P3.26 configuré en sortie
  initTimer0( 999 , 499 ) ;       // PR et MR pour interuption tout les 20 ms
  initTimer1( 999, direction ) ;       // PR et MR pour interuption tout les 20 ms
  while(1); //boucle infinie vide
  return 0 ;
}
