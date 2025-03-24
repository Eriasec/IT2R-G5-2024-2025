#include <stdio.h> #include <stdlib.h> #include "lpc17xx.h" #include "type.h" #include "i2c.h" #include "GLCD.h" #include "CRIS_UTILS.h"

void NunChuck_translate_data(void); void NunChuck_print_data_init(void); void NunChuck_print_data(void);
void search_for_i2c_devices(void); #define  FI 1 // Use font index 16x24 #define PORT_USED 1

#define NUNCHUK_ADDRESS_SLAVE1 0xA4 #define NUNCHUK_ADDRESS_SLAVE2 0xA5

volatile int joy_x_axis; 
volatile int joy_y_axis; 
volatile int accel_x_axis; 
volatile int accel_y_axis; 
volatile int accel_z_axis; 
volatile int z_button = 0; 
volatile int c_button = 0;

extern volatile uint8_t I2CMasterBuffer[I2C_PORT_NUM][BUFSIZE]; // BUFSIZE=64 extern volatile uint8_t I2CSlaveBuffer[I2C_PORT_NUM][BUFSIZE];
extern volatile uint32_t I2CReadLength[I2C_PORT_NUM]; 
extern volatile uint32_t I2CWriteLength[I2C_PORT_NUM];

char text_buffer[8];
volatile uint8_t ack_received, ack_sent; // debugging only


// we'll use delay_dirty() as a software delay function; it should produce
// about a second when del=(1 << 24) or so of delay depending on CCLK; volatile uint32_t temp;
void delay_dirty( uint32_t del)
 
{
uint32_t i;
for ( i=0; i<del; i++) { temp = i; }
}

// Communication with the Nunchuk consists of two phases:
// -->phase 1: initialization phase (executed once) in which specific data
// are written to the Nunchuk;
// Essentially initialization consists of two write transactions,
// each of which writes a single byte to a register internal to
// the I2C slave ( reg[0xf0] = 0x55, reg[0xfb] = 0x00 ).
// -->phase 2: repeated read phase in which six data bytes are read
// again and again; each read phase consists of two transactions –
// a write transaction which sets the read address to zero, and a
// read transaction.
// NOTES:
// -- I2C0 only supports 'fast mode' that the NunChuck uses!
// -- When I2C0 is used the pin connections are: SDA=P0.27, SCL=P0.28
// -- When I2C1 is used the pin connections are: SDA=P0.19, SCL=P0.20 void NunChuck_phase1_init(void)
{
// this function should be called once only; uint32_t i;

I2CWriteLength[PORT_USED] = 3; // write 3 bytes I2CReadLength[PORT_USED] = 0; // read 0 bytes I2CMasterBuffer[PORT_USED][0] = NUNCHUK_ADDRESS_SLAVE1;
I2CMasterBuffer[PORT_USED][1] = 0xF0; // at adress 0xF0 of NunChuck write: I2CMasterBuffer[PORT_USED][2] = 0x55; // data 0x55
I2CEngine( PORT_USED );

// should I introduce a delay? people say it's useful when debugging; delay_dirty( 0x100000 );

I2CWriteLength[PORT_USED] = 3; // write 3 bytes I2CReadLength[PORT_USED] = 0; // read 0 bytes I2CMasterBuffer[PORT_USED][0] = NUNCHUK_ADDRESS_SLAVE1;
I2CMasterBuffer[PORT_USED][1] = 0xFB; // at adress 0xFB of NunChuck write: I2CMasterBuffer[PORT_USED][2] = 0x00; // data 0x00
I2CEngine( PORT_USED );
}

void NunChuck_phase2_read(void)
{
// this is called repeatedly to realize continued polling of NunChuck uint32_t i;

I2CWriteLength[PORT_USED] = 2; // write 2 bytes I2CReadLength[PORT_USED] = 0; // read 6 bytes; I2CMasterBuffer[PORT_USED][0] = NUNCHUK_ADDRESS_SLAVE1;
I2CMasterBuffer[PORT_USED][1] = 0x00; // address I2CEngine( PORT_USED );

delay_dirty( 0x10000 );

I2CWriteLength[PORT_USED] = 1; // write 1 bytes I2CReadLength[PORT_USED] = 6; // read 6 bytes; I2CMasterBuffer[PORT_USED][0] = NUNCHUK_ADDRESS_SLAVE2; I2CEngine( PORT_USED );
// when I2CEngine() is executed, 6 bytes will be read and placed
// into I2CSlaveBuffer[][]
 
}


int main(void)
{
    uint32_t i;
    uint32_t x_prev=160, y_prev=120; uint32_t x_new=160, y_new=120;
    uint32_t dx=4, dy=4, delta=5, radius=16;

// (1) Initializations of GLCD and SER; GLCD_Init();  GLCD_SetTextColor(Yellow); CRIS_draw_circle(160,120, radius);

NunChuck_print_data_init();

// (2) SystemClockUpdate() updates the SystemFrequency variable SystemClockUpdate();

// (3) Init I2C devices; though we'll use only I2C1 I2C0Init( );
I2C1Init( ); I2C2Init( );

// (4)
    LPC_SC->PCONP |= ( 1 << 15 );
    LPC_GPIO0->FIODIR |= (1 << 21) | (1 << 22);
    LPC_GPIO0->FIOCLR |= 1 << 21;
    LPC_GPIO0->FIOSET |= 1 << 22;

// (5) NunChuck phase 1
//search_for_i2c_devices(); // debug only purposes; NunChuck_phase1_init();

// Note: Be careful with dirty fixed delays realized with for loops
// From device to device, or even same device with different write length,
// or various I2C clocks, such delay may need to be changed;
// however, it's good to have a break point between phases;
// (6) for ever loop while( 1 ) {

// (a) reset stuff
    for ( i = 0; i < BUFSIZE; i++ ) {
        I2CSlaveBuffer[PORT_USED][i] = 0x00;
    }

// (b) NunChuck phase 2 NunChuck_phase2_read(); NunChuck_translate_data(); NunChuck_print_data();

// (c) re-draw the circle to mimic movement if necessary;
// Note: joy_x_axis, joy_y_axis have values in range: 30..230
// with mid-range value of about 130 when the joystick rests;
// implement the simplest method to move the circle around:
// ->whenever joy_x_axis=190..230 (upper values in its range)
// keep shifting the circle to the right;
// ->whenever joy_x_axis=30..70 (lower values in its range)
// keep shifting the circle to the left;
// ->whenever joy_x_axis is in the mid-range do not move circle
 
// apply same logic for joy_x_axis
// TODO (assignments): move the circle based on the rotations
// i.e., do not use joystick; use buttons Z and C to increase
// or decrease the radius of the circle displayed on LCD;
if (joy_x_axis > 190) { dx = delta;
} else if (joy_x_axis < 90) { dx = -delta;
} else {
dx = 0;
}
x_new = x_prev + dx;
if (x_new > 320 - radius) x_new = 320 - radius; if (x_new < 0 + radius) x_new = radius;
if (joy_y_axis > 190) { dy = -delta;
} else if (joy_y_axis < 90) { dy = delta;
} else {
dy = 0;
}
y_new = y_prev + dy;
if (y_new > 240 - radius) y_new = 240 - radius; if (y_new < 0 + radius) y_new = radius;

if ( (x_new != x_prev) || (y_new != y_prev)) { // must move circle;
// first erase the circle at previous location; GLCD_SetTextColor(Black); CRIS_draw_circle(x_prev,y_prev, radius);
// then re-draw at new location; GLCD_SetTextColor(Yellow); CRIS_draw_circle(x_new,y_new, radius);
}
x_prev = x_new; y_prev = y_new;

// (d) long delay such that I have enough time to release joystick
// and have the circle stay at new location; this is a hack
// and should be modified to work nicely and to use rotations; delay_dirty( 0x10000 );
}

}


void NunChuck_translate_data(void)
{
int byte5 = I2CSlaveBuffer[PORT_USED][5]; joy_x_axis = I2CSlaveBuffer[PORT_USED][0]; joy_y_axis = I2CSlaveBuffer[PORT_USED][1]; accel_x_axis = (I2CSlaveBuffer[PORT_USED][2] << 2); accel_y_axis = (I2CSlaveBuffer[PORT_USED][3] << 2); accel_z_axis = (I2CSlaveBuffer[PORT_USED][4] << 2); z_button = 0;
c_button = 0;

// byte I2CSlaveBuffer[PORT_USED][5] contains bits for z and c buttons
// it also contains the least significant bits for the accelerometer data if ((byte5 >> 0) & 1)
z_button = 1;
if ((byte5 >> 1) & 1)
 
c_button = 1;
accel_x_axis += (byte5 >> 2) & 0x03; accel_y_axis += (byte5 >> 4) & 0x03; accel_z_axis += (byte5 >> 6) & 0x03;
}

void NunChuck_print_data_init(void)
{
// this should be called once only; GLCD_SetTextColor(White);
GLCD_Clear(Black); // clear graphical LCD display; set all pixels to Black GLCD_SetBackColor(Black); // set background color for when characters/text is printed GLCD_SetTextColor(White);
GLCD_DisplayString(0, 0,  FI, " This is I2C example"); GLCD_DisplayString(1, 0,  FI, " Data from NunChuck:");

GLCD_DisplayString(2, 2,  FI, "joyX ="); GLCD_DisplayString(3, 2,  FI, "joyY ="); GLCD_DisplayString(4, 2,  FI, "accX ="); GLCD_DisplayString(5, 2,  FI, "accY ="); GLCD_DisplayString(6, 2,  FI, "accZ ="); GLCD_DisplayString(7, 2,  FI, "Z ="); GLCD_DisplayString(8, 2,  FI, "C  =");
}

void NunChuck_print_data(void)
{
// this is called as many times as reads from the NunChuck; GLCD_SetTextColor(White);
sprintf(text_buffer, "%03d", joy_x_axis); GLCD_DisplayString(2, 10,  FI, (uint8_t*)text_buffer); sprintf(text_buffer, "%03d", joy_y_axis); GLCD_DisplayString(3, 10,  FI, (uint8_t*)text_buffer); sprintf(text_buffer, "%04d", accel_x_axis); GLCD_DisplayString(4, 10,  FI, (uint8_t*)text_buffer); sprintf(text_buffer, "%04d", accel_y_axis); GLCD_DisplayString(5, 10,  FI, (uint8_t*)text_buffer); sprintf(text_buffer, "%04d", accel_z_axis); GLCD_DisplayString(6, 10,  FI, (uint8_t*)text_buffer); sprintf(text_buffer, "%01d", z_button); GLCD_DisplayString(7, 10,  FI, (uint8_t*)text_buffer); sprintf(text_buffer, "%01d", c_button); GLCD_DisplayString(8, 10,  FI, (uint8_t*)text_buffer);
}
