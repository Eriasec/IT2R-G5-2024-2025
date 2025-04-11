#include "Driver_USART.h"               // ::CMSIS Driver:USART
#include "Driver_CAN.h"                 // ::CMSIS Driver:CAN
#include <stdio.h>
#include <string.h>
#include "cmsis_os.h"                   // ARM::CMSIS:RTOS:Keil RTX
#define osObjectsPublic                     // define objects in main module
#include "osObjects.h"                      // RTOS object definitions
#include "Driver_CAN.h"


#define GPS_BUF_TAILLE 300     
#define GPS_CAN_ID     0x5F8   // ID de la trame CAN


osThreadId id_GPS_CAN;


extern ARM_DRIVER_USART Driver_USART3; // USART3 pour réception d'une trame - Broches PD8/9
extern ARM_DRIVER_USART Driver_USART2; // USART2 pour renvoi trame traitée pour test - Broches PA2/3
extern ARM_DRIVER_CAN   Driver_CAN2;   // CAN2 pour la transmission vers la carte LCD - Broches PB12/13


char gps_buffer[GPS_BUF_TAILLE];  // Trame reçue
volatile int gps_data_ready = 0;  // Flag mis à 1 lorsque la trame est complètement reçue / volatile pour le compilateur

float latitude ;   // Latitude : ddmm.mmmm
float longitude ;  // Longitude : dddmm.mmmm
float time_utc;      // UTC TIME : hhmmss.sss
char status;  // "A" si DATA est valide ou "V" si invalide
char NS;      // NORD ou SUD : "N" ou "S"
char EW;      // EST ou OUEST : "E" ou "W"
char debugBuf[100];


void GPS_UART_Callback(uint32_t event) {
    if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {
        
        osSignalSet(id_GPS_CAN,0x01);
    }
}


void myCAN2_callback(uint32_t obj_idx, uint32_t event) {
    if (event & ARM_CAN_EVENT_SEND_COMPLETE) {
        
    }
}


void Init_UART_GPS(void) {     // Initialisation UART3 pour la réception GPS
    Driver_USART3.Initialize(GPS_UART_Callback);
    Driver_USART3.PowerControl(ARM_POWER_FULL);
    Driver_USART3.Control(ARM_USART_MODE_ASYNCHRONOUS |
                          ARM_USART_DATA_BITS_8 |
                          ARM_USART_STOP_BITS_1 |
                          ARM_USART_PARITY_NONE |
                          ARM_USART_FLOW_CONTROL_NONE,
                          9600);
    Driver_USART3.Control(ARM_USART_CONTROL_TX, 1);
    Driver_USART3.Control(ARM_USART_CONTROL_RX, 1);
    Driver_USART3.Receive(gps_buffer, GPS_BUF_TAILLE);
}


void Init_UART_Debug(void) {   // Initialisation UART2 pour renvoyer des données pour debug
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


void InitCan2(void) {     // Initialisation CAN2 pour transmettre les données exploitables vers le LCD
    Driver_CAN2.Initialize(NULL, myCAN2_callback);
    Driver_CAN2.PowerControl(ARM_POWER_FULL);
    Driver_CAN2.SetMode(ARM_CAN_MODE_INITIALIZATION);

    Driver_CAN2.SetBitrate(ARM_CAN_BITRATE_NOMINAL,
        125000,
        ARM_CAN_BIT_PROP_SEG(5U) |
        ARM_CAN_BIT_PHASE_SEG1(1U) |
        ARM_CAN_BIT_PHASE_SEG2(1U) |
        ARM_CAN_BIT_SJW(1U));

    Driver_CAN2.ObjectConfigure(2, ARM_CAN_OBJ_TX); // Objet pour transmission sur STM -> "2" - ATTENTION
    Driver_CAN2.SetMode(ARM_CAN_MODE_NORMAL);
}


void GPS_CAN_Thread(void const *arg) {   // Tâche reception UART du GPS - Transmission CAN vers LCD
    char* debut_trame;
    int champs_valides;
    uint8_t data_buf[8];
    ARM_CAN_MSG_INFO tx_msg_info;

    tx_msg_info.id  = ARM_CAN_STANDARD_ID(GPS_CAN_ID);
    tx_msg_info.rtr = 0;

    while (1) {
        
            osSignalWait(0x01, osWaitForever);		// sommeil, en attente de reception
            debut_trame = strstr(gps_buffer, "$GPRMC"); // Recherche de la trame GPRMC dans le buffer, si on la trouve, on pointe à l'adresse de la case correspondante

            if (debut_trame != NULL) {
                champs_valides = sscanf(debut_trame, "$GPRMC,%f,%c,%f,%c,%f,%c",  // On scanne les données
                                        &time_utc, &status,
                                        &latitude, &NS,
                                        &longitude, &EW);

                if (champs_valides == 6 && status == 'A') {
                    
                    sprintf(debugBuf, "%.4f", latitude);  //Lon=%.4f\r\n
                    

                    memcpy(&data_buf[0], &latitude, sizeof latitude);
                    memcpy(&data_buf[4], &longitude, sizeof longitude);
                  while (Driver_USART2.GetStatus().tx_busy);
                    Driver_USART2.Send(data_buf, 8);
                  
                    Driver_CAN2.MessageSend(2, &tx_msg_info, data_buf, 8); // On met debugBuf pour tester directement avec la latitude
                    //Driver_CAN2.MessageSend(2, &tx_msg_info, (const uint8_t *)&latitude, 4);
                } else {
                    sprintf(debugBuf, "Trame invalide\r\n");
                    while (Driver_USART2.GetStatus().tx_busy);
                    Driver_USART2.Send(debugBuf, strlen(debugBuf));
                }
            } else {
                sprintf(debugBuf, "Pas de $GPRMC\r\n");  // Envoi message d'erreur si trame pas trouvée
                while (Driver_USART2.GetStatus().tx_busy);
                Driver_USART2.Send(debugBuf, strlen(debugBuf));
            }

            Driver_USART3.Receive(gps_buffer, GPS_BUF_TAILLE); 
        }

         
    }

osThreadDef(GPS_CAN_Thread, osPriorityNormal, 1, 0);


int main(void) {
    osKernelInitialize();

    Init_UART_GPS();    // Initialisation UART3 pour le module GPS
    Init_UART_Debug();  
    InitCan2();         

    id_GPS_CAN = osThreadCreate(osThread(GPS_CAN_Thread), NULL);

    osKernelStart();
    osDelay(osWaitForever);
}