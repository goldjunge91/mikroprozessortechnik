//===========================================================
// Test program UART6 TX 8/N/1 @ 115200 bit via Port PP1
// LTL 17.5.2020 / mod. V0.2 - V0.4 RMS 1.6.2023
//==========================================================
// #include "inc/tm4c1294ncpdt.h" 			// Header of the controller type
#include "tm4c1294ncpdt.h"
#include <stdint.h> 				// Header w. types for the register ..
#define IDLETIME 1000 				// waiting time between transmissions
// #define BPS 38400
static int wait_counter_1b = 0;

void config_port_1b(void){
    // initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000; 		// switch on clock for Port P
    wait_counter_1b++;                  // delay for stable clock
    // initialize Port P
    GPIO_PORTP_DEN_R |= 0x2; 			// enable digital pin function for PP1
    GPIO_PORTP_DIR_R |= 0x2; 			// set PP1 to output
    GPIO_PORTP_AFSEL_R |= 0x2; 			// switch to alternate pin function PP1
    GPIO_PORTP_PCTL_R |= 0x10; 			// select alternate pin function PP1->U6Tx
}

void config_uart_1b(uint32_t baudrate){
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40; 			            // switch on clock for UART6
    wait_counter_1b++; 					            // delay for stable clock
    UART6_CTL_R &= ~0x01; 			                // disable UART6 for config
    // initialize bitrate of 115200 bit per second
    UART6_IBRD_R = 16000000/(16*baudrate); 		    // set DIVINT of BRD floor(16 MHz/16*9600 bps)
    UART6_FBRD_R = (16000000/(16*baudrate))*64; 	// set DIVFRAC of BRD remaining fraction divider
    UART6_LCRH_R = 0x00000062; 			            // serial format 8N1
    UART6_CTL_R |= 0x0101; 			                // UART transmit on and UART enable
}

void idle_1b() { 					// simple wait for idle state
    int i;
    for (i=IDLETIME;i>0;i--); 			// count down loop for waiting
}

/* @describe
*/
void run_aufgabe_1b(char zeichen, uint32_t baudrate){
    config_port_1b(); 				// configuration of Port P
    config_uart_1b(baudrate); 				// configuration of UART6
    while(1){
        // https://www.geeksforgeeks.org/embedded-c/
        while((UART6_FR_R & 0x20) !=0); // Explizit und Sicher	// till transmit FIFO not full 
        UART6_DR_R = zeichen;			// send the character from zeichen
        idle_1b();
    }
}

void init_aufgabe_1b(void) {
    config_port_1b();
}

void execute_aufgabe_1b(char zeichen, uint32_t baudrate) {
    // Statische Variable, um sicherzustellen, dass die UART nur einmal konfiguriert wird
    static uint32_t last_baudrate = 0;
    if (baudrate != last_baudrate) {
        config_uart_1b(baudrate);
        last_baudrate = baudrate;
    }
    while((UART6_FR_R & 0x20) !=0); // https://www.geeksforgeeks.org/embedded-c/
    UART6_DR_R = zeichen;
    idle_1b();
}
