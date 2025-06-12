//===========================================================
// Aufgabe 1b: UART6 TX Configuration
// Requirements: ASCII character 'a', 38400 bps, 8O1 format (8 data bits, Odd Parity, 1 Stop bit)
//               Transmission via Port PP1 (U6TX)
//===========================================================
#include "aufgaben.h"
#include "tm4c1294ncpdt.h"
#include <stdint.h> 				// Header with types for the registers

#define IDLETIME 1000 				// Waiting time between transmissions
#define SYSTEM_CLOCK 16000000U      // System clock in Hz (I'm assuming 16 MHz without PLL)

static volatile int wait_counter_1b = 0; // 'volatile' to prevent compiler optimization if used as a simple delay loop

/**
 * @brief Configures Port P for UART6 TX functionality.
 *
 * Activates the clock for Port P and configures Pin PP1 (Port P, Pin 1)
 * as an alternate function U6TX. Pin PP1 is configured as a digital output
 * with the alternate function for UART6 Transmit enabled.
 */
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

/**
 * @brief Configures UART6 for serial communication.
 *
 * @param baudrate The desired baud rate for UART communication (e.g., 38400).
 *                 Configured for 801 format (8 data bits, Odd Parity, 1 Stop bit).
 *                 FIFOs are disabled according to the LCRH setting (0x62).
 */
void config_uart_1b(uint32_t baudrate){
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40; 			            // switch on clock for UART6
    // wait_counter_1b++; 					            // delay for stable clock
    while ((SYSCTL_PRUART_R & 0x40) == 0) {
    }; // Wait until bit 6 of UART6 (PRUART) is set. Bit-shift //  while((SYSCTL_PRUART_R & (1U << 6)) == 0) {};
    UART6_CTL_R &= ~0x01; 			                // disable UART6 for config
    // initialize bitrate of 115200 bit per second
    UART6_IBRD_R = 16000000/(16*baudrate); 		    // set DIVINT of BRD floor(16 MHz/16*9600 bps)
    UART6_FBRD_R = (16000000/(16*baudrate))*64; 	// set DIVFRAC of BRD remaining fraction divider
        
    // Configure serial parameters: 8 data bits, Odd Parity, 1 Stop bit (8O1), FIFO disabled
    // UART6_LCRH_R:
    // WLEN (Bits 6:5) = 0b11 (8 data bits)
    // FEN  (Bit 4)    = 0 (FIFO disabled)
    // STP2 (Bit 3)    = 0 (1 Stop bit)
    // EPS  (Bit 2)    = 0 (Odd Parity Select)
    // PEN  (Bit 1)    = 1 (Parity Enable)
    // Value: (0x3 << 5) | (1 << 1) = 0x60 | 0x02 = 0x62
    UART6_LCRH_R = 0x00000062; // serial format 801
    
    // Enable UART6 and Transmit
    // UART6_CTL_R:
    // TXE (Bit 8)    = 1 (Transmit Enable)
    // UARTEN (Bit 0) = 1 (UART Enable)
    // Value: (1 << 8) | (1 << 0) = 0x100 | 0x01 = 0x101
    UART6_CTL_R |= 0x0101; 	
}

/**
 * @brief Creates a simple delay using a counting loop.
 *
 * The exact duration depends on the clock frequency and compiler optimizations.
 * 'volatile' is used for the loop variable to prevent the compiler
 * from optimizing away the loop.
 */
void idle_1b() {
    volatile int i; // 'volatile' to prevent compiler optimization
    for (i = IDLETIME; i > 0; i--); 			// Countdown loop for waiting
}

/**
 * @brief Repeatedly sends a character via UART6 at the specified baud rate.
 *        This function contains an infinite loop.
 *
 * @param zeichen The character to send.
 * @param baudrate The baud rate for UART transmission.
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

/**
 * @brief Initializes the necessary hardware (Port P) for Aufgabe 1b.
 *        UART configuration is done in `execute_aufgabe_1b`.
 */
void init_aufgabe_1b(void) {
    config_port_1b();
}

/**
 * @brief Sends a single character via UART6.
 *
 * Configures UART6 on the first execution or if the baud rate changes.
 * Waits until the transmit FIFO has space, sends the character, and then waits for a defined time.
 *
 * @param zeichen The character to send.
 * @param baudrate The baud rate to use.
 */
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
