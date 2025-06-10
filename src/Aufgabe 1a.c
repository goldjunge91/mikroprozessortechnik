//===========================================================
// Test program UART6 TX 8/N/1 @ 115200 bit via Port PP1
// LTL 17.5.2020 / mod. V0.2 - V0.4 RMS 1.6.2023
//==========================================================
#include "aufgaben.h"
#include "inc/tm4c1294ncpdt.h" 			// Header of the controller type
#include <stdint.h> 				// Header w. types for the register ..

#define IDLETIME 1000 				// waiting time between transmissions
#define BPS 9600
static int wt_1a = 0; 					// auxillary variable
void config_port_1a(void){
    						// initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000; 		// switch on clock for Port P
    wt_1a++; // delay for stable clock
    // initialize Port P
    GPIO_PORTP_DEN_R |= 0x2; 			// enable digital pin function for PP1
    GPIO_PORTP_DIR_R |= 0x2; 			// set PP1 to output
    GPIO_PORTP_AFSEL_R |= 0x2; 			// switch to alternate pin function PP1
    GPIO_PORTP_PCTL_R |= 0x10; 			// select alternate pin function PP1->U6Tx
}
//void config_uart_1a(int bps){
void config_uart_1a(){
    						// initialize UART6
    SYSCTL_RCGCUART_R |= 0x40; 			// switch on clock for UART6
    wt_1a++; 					// delay for stable clock
    UART6_CTL_R &= ~0x01; 			// disable UART6 for config
    // initialize bitrate of 115200 bit per second
    UART6_IBRD_R = 16000000/(16*BPS); 		// set DIVINT of BRD floor(16 MHz/16*9600 bps)
    UART6_FBRD_R = (16000000/(16*BPS))*64; 	// set DIVFRAC of BRD remaining fraction divider (16 MHz/16*9600 bps)*64 = 6656
    UART6_LCRH_R = 0x00000046; 			// serial format 7E1
    UART6_CTL_R |= 0x0101; 			// UART transmit on and UART enable
}
void idle() { 					// simple wait for idle state
    int i;
    for (i=IDLETIME;i>0;i--); 			// count down loop for waiting
}
//void run_aufgabe_1a(char character, int baudrate){
void run_aufgabe_1a(){
    config_port_1a(); 				// configuration of Port P
    config_uart_1a(); 				// configuration of UART6
    while(1){
        while((UART6_FR_R & 0x20) !=0); 	// till transmit FIFO not full
        printf("Aufgabe 1a\n");
        UART6_DR_R = 'X';   // send the character 'X'
        idle(); 				// idle time
    }
}
