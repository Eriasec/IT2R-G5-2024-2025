#include "Driver_I2C.h"                 // ARM::CMSIS Driver:I2C:Custom
#include "stm32f4xx.h"                  // Device header
#include "stm32f4xx_hal.h"              // Keil::Device:STM32Cube HAL:Common



#define SLAVE_I2C_ADDR 0x52              // Adresse esclave du Nunchuk en 7 bits

extern ARM_DRIVER_I2C Driver_I2C1;     // D�claration de la structure I2C1

volatile int joy_x_axe;
volatile int joy_y_axe;
volatile int acc_x_axe;
volatile int acc_y_axe;
volatile int acc_z_axe;
volatile int z_button = 0;
volatile int c_button = 0;

void Init_I2C(void);
void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur);
unsigned char read1byte(unsigned char composant, unsigned char registre);
void NunChuck_phase1_init(void);
void NunChuck_phase2_read(void);
void NunChuck_translate_data(void);

int main(void) {
    Init_I2C(); // Initialisation de l'I2C
    NunChuck_phase1_init(); // Initialisation du Nunchuk

    while (1) {
        NunChuck_phase2_read(); // Lire les donn�es du Nunchuk
        NunChuck_translate_data(); // Traduire les donn�es
    }
}

void Init_I2C(void) {
    Driver_I2C1.Initialize(NULL);
    Driver_I2C1.PowerControl(ARM_POWER_FULL);
    Driver_I2C1.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
}

void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur) {
    unsigned char tab[2];
    tab[0] = registre;
    tab[1] = valeur;

    Driver_I2C1.MasterTransmit(composant >> 1, tab, 2, false);
    while (Driver_I2C1.GetStatus().busy == 1);
}

unsigned char read1byte(unsigned char composant, unsigned char registre) {
    uint8_t maValeur;
    unsigned char tab[1];
    tab[0] = registre;

    Driver_I2C1.MasterTransmit(composant >> 1, tab, 1, true);
    while (Driver_I2C1.GetStatus().busy == 1);

    Driver_I2C1.MasterReceive(composant >> 1, &maValeur, 1, false);
    while (Driver_I2C1.GetStatus().busy == 1);

    return maValeur;
}

void NunChuck_phase1_init(void) {
    write1byte(SLAVE_I2C_ADDR, 0xF0, 0x55);
    write1byte(SLAVE_I2C_ADDR, 0xFB, 0x00);
}

void NunChuck_phase2_read(void) {
    read1byte(SLAVE_I2C_ADDR, 0xFB);
}

void NunChuck_translate_data(void) {
    unsigned char byte5 = read1byte(SLAVE_I2C_ADDR, 0x05); //  lit un octet du registre 0x05 du Nunchuk (qui contient des informations de contr�le pour les boutons Z et C ainsi que des donn�es pour les axes d'acc�l�ration).

    joy_x_axe = read1byte(SLAVE_I2C_ADDR, 0x00); // lecture du registre 0x00 pour l'axe X (valeurs comprises entre 0 et 255)
    joy_y_axe = read1byte(SLAVE_I2C_ADDR, 0x01); //lecture du registre 0x00 pour l'axe Y (valeurs comprises entre 0 et 255)
    acc_x_axe = (read1byte(SLAVE_I2C_ADDR, 0x02) << 2); //lecture du registre 0x02 pour l'acc�l�rateur X 
    acc_y_axe = (read1byte(SLAVE_I2C_ADDR, 0x03) << 2); //lecture du registre 0x03 pour l'acc�l�rateur Y 
    acc_z_axe = (read1byte(SLAVE_I2C_ADDR, 0x04) << 2); //lecture du registre 0x04 pour l'acc�l�rateur Z 

    z_button = (byte5 >> 0) & 1; // extraction bouton Z
    c_button = (byte5 >> 1) & 1; // extraction bouton C
		
    acc_x_axe += (byte5 >> 2) & 0x03;
    acc_y_axe += (byte5 >> 4) & 0x03;
    acc_z_axe += (byte5 >> 6) & 0x03;
}




//// Variables pour les donn�es du Nunchuk
//volatile int joy_x_axe;
//volatile int joy_y_axe;
//volatile int acc_x_axe;
//volatile int acc_y_axe;
//volatile int acc_z_axe;
//volatile int z_button = 0;
//volatile int c_button = 0;

//// Fonctions pour l'initialisation I2C, �criture et lecture de donn�es
//void Init_I2C(void);
//void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur);
//unsigned char read1byte(unsigned char composant, unsigned char registre);
//void NunChuck_phase1_init(void);
//void NunChuck_phase2_read(void);
//void NunChuck_translate_data(void);

//int main(void) {
//    Init_I2C(); // Initialisation de l'I2C

//    // Initialisation du Nunchuk : phase 1
//    NunChuck_phase1_init();

//    while (1) {
//        // Phase 2 : Lire les donn�es du Nunchuk
//        NunChuck_phase2_read();
//        
//        // Traduire les donn�es
//        NunChuck_translate_data();
//        
//        // Traitement des donn�es du Nunchuk (Joystick et Acc�l�rateur)
//        // Par exemple, afficher les valeurs du joystick et des boutons
//        // Tu peux remplacer cette partie par des traitements suppl�mentaires
//    }
//}

//// Fonction pour initialiser l'I2C
//void Init_I2C(void) {
//    Driver_I2C1.Initialize(NULL);  // Initialisation de l'I2C
//    Driver_I2C1.PowerControl(ARM_POWER_FULL);  // Activation de l'alimentation du p�riph�rique
//    Driver_I2C1.Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);  // D�finir la vitesse � 100kHz
//}

//// Fonction pour �crire un octet dans un registre
//void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur) {
//    unsigned char tab[2];
//    tab[0] = registre;  // Registre de destination
//    tab[1] = valeur;    // Valeur � �crire

//    Driver_I2C1.MasterTransmit(composant >> 1, tab, 2, false);  // Transmission I2C
//    while (Driver_I2C1.GetStatus().busy == 1);  // Attente que la transmission soit termin�e
//}

//// Fonction pour lire un octet d'un registre
//unsigned char read1byte(unsigned char composant, unsigned char registre) {
//    uint8_t maValeur;
//    unsigned char tab[1];
//    tab[0] = registre;  // Registre de destination

//    Driver_I2C1.MasterTransmit(composant >> 1, tab, 1, true);  // Transmission I2C pour envoyer l'adresse du registre
//    while (Driver_I2C1.GetStatus().busy == 1);  // Attente de la fin de la transmission

//    Driver_I2C1.MasterReceive(composant >> 1, &maValeur, 1, false);  // R�ception des donn�es
//    while (Driver_I2C1.GetStatus().busy == 1);  // Attente que la r�ception soit termin�e

//    return maValeur;
//}

//// Fonction d'initialisation de la phase 1 du Nunchuk (initialisation I2C)
//void NunChuck_phase1_init(void) {
//    // Envoi des deux commandes pour initialiser le Nunchuk
//    write1byte(SLAVE_I2C_ADDR, 0xF0, 0x55);  // Commande d'initialisation
//    write1byte(SLAVE_I2C_ADDR, 0xFB, 0x00);  // Autre commande d'initialisation
//}

//// Fonction pour lire les donn�es du Nunchuk (phase 2)
//void NunChuck_phase2_read(void) {
//    // Lire les 6 octets de donn�es du Nunchuk
//    read1byte(SLAVE_I2C_ADDR, 0x00);  // Lecture des donn�es de la premi�re position
//}

//// Traduction des donn�es du Nunchuk pour les utiliser
//void NunChuck_translate_data(void) {
//    unsigned char byte5 = read1byte(SLAVE_I2C_ADDR, 0x05);

//    joy_x_axe = read1byte(SLAVE_I2C_ADDR, 0x00);  // Joystick X
//    joy_y_axe = read1byte(SLAVE_I2C_ADDR, 0x01);  // Joystick Y
//    acc_x_axe = (read1byte(SLAVE_I2C_ADDR, 0x02) << 2);  // Acc�l�rateur X
//    acc_y_axe = (read1byte(SLAVE_I2C_ADDR, 0x03) << 2);  // Acc�l�rateur Y
//    acc_z_axe = (read1byte(SLAVE_I2C_ADDR, 0x04) << 2);  // Acc�l�rateur Z

//    // V�rification des boutons Z et C
//    z_button = (byte5 >> 0) & 1;
//    c_button = (byte5 >> 1) & 1;
//    acc_x_axe += (byte5 >> 2) & 0x03;
//    acc_y_axe += (byte5 >> 4) & 0x03;
//    acc_z_axe += (byte5 >> 6) & 0x03;
//}










//unsigned char read1byte(unsigned char composant, unsigned char registre)
//{
//	uint8_t maValeur;
//	unsigned char tab[1];
//	tab[0] = registre;
//	Driver_I2C1.MasterTransmit (composant,tab, 1, true);
//	while (Driver_I2C1.GetStatus().busy == 1);
//	
//	Driver_I2C1.MasterReceive (composant,&maValeur, 1, false);
//	while (Driver_I2C1.GetStatus().busy == 1);
//	
//}

//void write1byte(unsigned char composant, unsigned char registre, unsigned char valeur)
//{
//	unsigned char tab[2];
//	tab[0] = registre;
//	tab[1] = valeur;
//	
//	Driver_I2C1.MasterTransmit (composant,tab, 2, false);
//	while (Driver_I2C1.GetStatus().busy == 1);
//	
//}

//void Init_I2C(void){ 
//	
//	Driver_I2C1.Initialize(NULL); // d�but initialisation (on fera mieux bient�t!)
//	Driver_I2C1.PowerControl(ARM_POWER_FULL); // alimentation p�riph�rique
//	Driver_I2C1.Control( ARM_I2C_BUS_SPEED, // 2nd argument = d�bit
//	ARM_I2C_BUS_SPEED_STANDARD ); // =100 kHz

//}


//while (1)
//  {
//		write1byte(SLAVE_I2C_ADDR,0xF0,0x55);
//		write1byte(SLAVE_I2C_ADDR,0xFB,0x00);
//		read1byte(SLAVE_I2C_ADDR,0xFB);
//  }


