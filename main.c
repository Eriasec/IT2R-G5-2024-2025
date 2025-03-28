#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include <stdio.h>
#include <string.h>
extern ARM_DRIVER_USART Driver_USART3; // USART3 pour r�ception d'une trame - Broche PD8/9
extern ARM_DRIVER_USART Driver_USART2; // USART2 pour renvoi trame trait�e pour test - Broche PA2/3

#define GPS_BUF_TAILLE 300  // Pour faciliter les changements 


char gps_buffer[GPS_BUF_TAILLE];       // Trame re�ue

float time_utc;           // UTC TIME : hhmmss.sss
char status;              // "A" si DATA est valide ou "V" si invalide
float latitude;           // Latitude : ddmm.mmmm
char NS;                  // NORD ou SUD : "N" ou "S"
float longitude;          // Longitude : dddmm.mmmm
char EW;                  // EST ou OUEST : "E" ou "W"


volatile int gps_data_ready = 0;  // Flag mis � 1 lorsque la trame est compl�tement re�ue / volatile pour le compilateur


void GPS_UART_Callback(uint32_t event) {     // fonction Callback appel�e lorsque la trame est re�ue et exploitable
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        gps_data_ready = 1;                  // Signale que la trame est pr�te � �tre trait�e
    }
}
  
void Init_UART_GPS(void) {
    Driver_USART3.Initialize(GPS_UART_Callback);    // Initialisation de l�UART3 avec la fonction callback
    Driver_USART3.PowerControl(ARM_POWER_FULL);
    Driver_USART3.Control(ARM_USART_MODE_ASYNCHRONOUS |
                          ARM_USART_DATA_BITS_8 |
                          ARM_USART_STOP_BITS_1 |
                          ARM_USART_PARITY_NONE |
                          ARM_USART_FLOW_CONTROL_NONE,
                          9600);
    Driver_USART3.Control(ARM_USART_CONTROL_TX, 1);
    Driver_USART3.Control(ARM_USART_CONTROL_RX, 1);

    Driver_USART3.Receive(gps_buffer, GPS_BUF_TAILLE);  // On d�marre la r�ception de la trame
}


void Init_UART_Debug(void) {              // Initialisation de l�UART2 pour debug
    Driver_USART2.Initialize(NULL);    
    Driver_USART2.PowerControl(ARM_POWER_FULL);
    Driver_USART2.Control(ARM_USART_MODE_ASYNCHRONOUS |
                          ARM_USART_DATA_BITS_8 |
                          ARM_USART_STOP_BITS_1 |
                          ARM_USART_PARITY_NONE |
                          ARM_USART_FLOW_CONTROL_NONE,
                          9600);
    Driver_USART2.Control(ARM_USART_CONTROL_TX, 1);
    Driver_USART2.Control(ARM_USART_CONTROL_RX, 1);
}


void GPS_TraiterTrame(void) {   // Fonction de traitement d�une trame GPRMC re�ue
    char* debut_trame;      
    char* fin_trame;  
    char debugBuf[200];      
    int champs_valides;      

    debut_trame = strstr(gps_buffer, "$GPRMC");   // Recherche de la trame GPRMC dans le buffer, si on la trouve, on pointe � l'adresse de la case correspondante
    if (debut_trame != NULL) {
       
        fin_trame = strstr(gps_buffer, "CRLF"); 
        if (fin_trame != NULL){
            sprintf(debugBuf, "pas de fin de trame $GPRMC\r\n");
        }
        champs_valides = sscanf(debut_trame, "$GPRMC,%f,%c,%f,%c,%f,%c",   // On scanne les donn�es
                                &time_utc, &status,
                                &latitude, &NS,
                                &longitude, &EW);

        if (champs_valides == 6) {
            sprintf(debugBuf, "valide=%d,lat=%.4f,%c,lon=%.4f,%c", champs_valides, latitude, NS, longitude, EW); // Pr�paration de l'affichage des donn�es valides pour debug
        } else {
            sprintf(debugBuf, "Trame invalide !\r\n");
        }
    } else {
        
        sprintf(debugBuf, "Pas de $GPRMC trouv�.\r\n");   // Envoi message d'erreur si trame pas trouv�e
    }

    
    while (Driver_USART2.GetStatus().tx_busy);
    Driver_USART2.Send(debugBuf, strlen(debugBuf));

    
    gps_data_ready = 0;        // On r�initialise pour relancer le cycle
    Driver_USART3.Receive(gps_buffer, GPS_BUF_TAILLE);
}


int main(void) {
    Init_UART_GPS();     // Initialisation UART avec le module GPS
    Init_UART_Debug();   // Initialisation UART pour debug

    while (1) {
        
        if (gps_data_ready) {   // Si la trame a �t� re�ue compl�tement
            GPS_TraiterTrame();  // On traite les donn�es
        }

        
    }
}
