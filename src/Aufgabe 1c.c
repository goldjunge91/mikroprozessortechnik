//===========================================================
// Aufgabe 1c: UART6 TX Configuration
// Requirements: Binary data 0x3B, 4800 bps, 7N2 format (7 data bits, No Parity, 2 Stop bits)
//               Transmission via Port PP1 (U6TX)
//===========================================================

// #include "inc/tm4c1294ncpdt.h" 			// Header of the controller type
#include "aufgaben.h"
#include "tm4c1294ncpdt.h"
#include <stdint.h>

#define IDLETIME 1000 				        // Waiting time between transmissions
#define SYSTEM_CLOCK 16000000U          // System clock in Hz (I'm assuming 16 MHz without PLL)

// This counter was likely intended for simple delays.
// It's better to wait for the peripheral ready flags (PRGPIO, PRUART).
static volatile int wait_counter_1c = 0; // 'volatile' to prevent compiler optimization if used as a simple delay loop

/**
 * @brief Configures Port P for UART6 TX functionality.
 *
 * Activates the clock for Port P and configures Pin PP1 (Port P, Pin 1)
 * as an alternate function U6TX. Pin PP1 is configured as a digital output
 * with the alternate function for UART6 Transmit enabled.
 */
void config_port_1c(void){
    						// initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000; 		// switch on clock for Port P
    // wait_counter_1c++; // delay for stable clock
    while ((SYSCTL_PRGPIO_R & 0x02000) == 0) {
    }; // Wait until bit 13 of Port P (PRGPIO) is set. Bit-shift //  while((SYSCTL_PRGPIO_R & (1U << 13)) == 0) {};

    // initialize Port P
    GPIO_PORTP_DEN_R |= 0x2; 			// enable digital pin function for PP1
    GPIO_PORTP_DIR_R |= 0x2; 			// set PP1 to output
    GPIO_PORTP_AFSEL_R |= 0x2; 			// switch to alternate pin function PP1
    GPIO_PORTP_PCTL_R |= 0x10; 			// select alternate pin function PP1->U6Tx
}

/**
 * @brief Configures UART6 for serial communication.
 *
 * @param baudrate The desired baud rate for UART communication (e.g., 4800 for this task).
 *                 Configured for 7N2 format (7 data bits, No Parity, 2 Stop bits).
 *                 FIFOs are disabled.
 */
void config_uart_1c(uint32_t baudrate){
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40; 			// switch on clock for UART6
    // wait_counter_1c++; 					// delay for stable clock
    while ((SYSCTL_PRUART_R & 0x40) == 0) {
    }; // Wait until bit 6 of UART6 (PRUART) is set. Bit-shift //  while((SYSCTL_PRUART_R & (1U << 6)) == 0) {};
    UART6_CTL_R &= ~0x01; 			// disable UART6 for config

    // initialize bitrate
    UART6_IBRD_R = SYSTEM_CLOCK/(16*baudrate); 		// set DIVINT of BRD floor(16 MHz/16*9600 bps)
    UART6_FBRD_R = (SYSTEM_CLOCK/(16*baudrate))%64; 	// set DIVFRAC of BRD remaining fraction divider

    // Configure serial parameters: 7 data bits, No Parity, 2 Stop bits (7N2), FIFO disabled
    // UART6_LCRH_R:
    // WLEN (Bits 6:5) = 0b10 (7 data bits)
    // FEN  (Bit 4)    = 0 (FIFO disabled)
    // STP2 (Bit 3)    = 1 (2 Stop bits)
    // PEN  (Bit 1)    = 0 (Parity disabled)
    // Value: (0x2 << 5) | (1 << 3) = 0x40 | 0x08 = 0x48
    UART6_LCRH_R = 0x00000048; 			// Serial format 7N2, FIFO disabled

    // Enable UART6 and Transmit
    // UART6_CTL_R:
    // TXE (Bit 8)    = 1 (Transmit Enable)
    // UARTEN (Bit 0) = 1 (UART Enable)
    // Value: (1 << 8) | (1 << 0) = 0x100 | 0x01 = 0x101
    UART6_CTL_R |= 0x0101; 			    // UART transmit on and UART enable
}

/**
 * @brief Creates a simple delay using a counting loop.
 *
 * The exact duration depends on the clock frequency and compiler optimizations.
 * 'volatile' is used for the loop variable to prevent the compiler
 * from optimizing away the loop.
 */
void idle_1c() {
    volatile int i; // 'volatile' to prevent compiler optimization
    for (i = IDLETIME; i > 0; i--); 			// Countdown loop for waiting
}

/**
 * @brief Repeatedly sends a character via UART6 at the specified baud rate.
 *        This function contains an infinite loop.
 *
 * @param zeichen The character (or binary data) to send.
 * @param baudrate The baud rate for UART transmission.
 */
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

/**
 * @brief Initializes the necessary hardware (Port P) for Aufgabe 1c.
 *        UART configuration is done in `execute_aufgabe_1c`.
 */
void init_aufgabe_1c(void) {
    config_port_1c();
}

/**
 * @brief Sends a single character (or binary data) via UART6.
 *
 * Configures UART6 on the first execution or if the baud rate changes.
 * Waits until the transmit FIFO has space, sends the character/data, and then waits for a defined time.
 *
 * @param zeichen The character (or binary data) to send.
 * @param baudrate The baud rate to use.
 */
void execute_aufgabe_1c(char zeichen, uint32_t baudrate) {
    // Statische Variable, um sicherzustellen, dass die UART nur einmal konfiguriert wird
    static uint32_t last_baudrate = 0;

    if (baudrate != last_baudrate) {
        config_uart_1c(baudrate);
        last_baudrate = baudrate;
    }
    // I'll wait here until the transmit FIFO is not full.
    while((UART6_FR_R & 0x20) != 0); // Loop as long as TXFF (Bit 5) is set (FIFO full)
    UART6_DR_R = zeichen; // Write character/data to data register to send
    idle_1c(); // Short pause after sending
}
