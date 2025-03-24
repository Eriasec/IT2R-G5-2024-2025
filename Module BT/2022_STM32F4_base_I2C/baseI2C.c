#include "Driver_I2C.h"                 // ::CMSIS Driver:I2C

#define SLAVE_I2C_ADDR       0xXX			// Adresse esclave sur 7 bits

extern ARM_DRIVER_I2C Driver_I2C0;

uint8_t DeviceAddr;

void Init_I2C(void){
	Driver_I2C0.Initialize(NULL);
	Driver_I2C0.PowerControl(ARM_POWER_FULL);
	Driver_I2C0.Control(	ARM_I2C_BUS_SPEED,				// 2nd argument = débit
							ARM_I2C_BUS_SPEED_STANDARD  );	// 100 kHz
	Driver_I2C0.Control(	ARM_I2C_BUS_CLEAR,
							0 );
}

int main (void){
	uint8_t tab[10], maValeur;
	
	Init_I2C();
	
	while (1)
	{
		tab[0] = YY;
		tab[1] = ZZ;
		// Ecriture vers registre esclave : START + ADDR(W) + 1W_DATA + 1W_DATA + STOP
		Driver_I2C0.MasterTransmit (SLAVE_I2C_ADDR, tab, 2, false);		// false = avec stop
		while (Driver_I2C0.GetStatus().busy == 1);	// attente fin transmission
		
		// Lecture de data esclave : START + ADDR(R) + 1R_DATA + STOP
		Driver_I2C0.MasterReceive (SLAVE_I2C_ADDR, &maValeur, 1, false);		// false = avec stop
		while (Driver_I2C0.GetStatus().busy == 1);	// attente fin transmission
	}
	
	return 0;
}
