//############################################################################
//###                                                                      ###
//###      AUFGABE 1c: Uart Senden von 'X', 38400 über UART6 von Port P1   ###
//###                                                                      ###
//############################################################################

// #include "inc/tm4c1294ncpdt.h" 			// Header of the controller type
#include "tm4c1294ncpdt.h"
#include <stdint.h>
#define IDLETIME 1000 				        // waiting time between transmissions
static int wait_counter_1c = 0;

void config_port_1c(void){
    						// initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000; 		// switch on clock for Port P
    wait_counter_1c++; // delay for stable clock
    // initialize Port P
    GPIO_PORTP_DEN_R |= 0x2; 			// enable digital pin function for PP1
    GPIO_PORTP_DIR_R |= 0x2; 			// set PP1 to output
    GPIO_PORTP_AFSEL_R |= 0x2; 			// switch to alternate pin function PP1
    GPIO_PORTP_PCTL_R |= 0x10; 			// select alternate pin function PP1->U6Tx
}
void config_uart_1c(uint32_t baudrate){
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40; 			// switch on clock for UART6
    wait_counter_1c++; 					// delay for stable clock
    UART6_CTL_R &= ~0x01; 			// disable UART6 for config
    						// initialize bitrate of 115200 bit per second
    UART6_IBRD_R = 16000000/(16*baudrate); 		// set DIVINT of BRD floor(16 MHz/16*9600 bps)
    UART6_FBRD_R = (16000000/(16*baudrate))%64; 	// set DIVFRAC of BRD remaining fraction divider
    UART6_LCRH_R = 0x00000048; 			// serial format 7N2
    UART6_CTL_R |= 0x0101; 			// UART transmit on and UART enable
}
void idle_1c() { 					// simple wait for idle state
    int i;
    for (i=IDLETIME;i>0;i--); 			// count down loop for waiting
}
void run_aufgabe_1c(char zeichen, uint32_t baudrate){
    config_port_1c(); 				// configuration of Port P
    config_uart_1c(baudrate); 				// configuration of UART6
    while(1){
        while((UART6_FR_R & 0x20) !=0); 	// till transmit FIFO not full
        // UART6_DR_R = 0x3B;			// send the character ';'
        UART6_DR_R = zeichen;			// send the character ';'
        idle_1c(); 				// idle time
    }
}

void init_aufgabe_1c(void) {
    config_port_1c();
}

void execute_aufgabe_1c(char zeichen, uint32_t baudrate) {
    // Statische Variable, um sicherzustellen, dass die UART nur einmal konfiguriert wird
    static uint32_t last_baudrate = 0;
    if (baudrate != last_baudrate) {
        config_uart_1c(baudrate);
        last_baudrate = baudrate;
    }
    // Ein Zeichen senden
    while((UART6_FR_R & 0x20) != 0);
    UART6_DR_R = zeichen;
    idle_1c();
}
