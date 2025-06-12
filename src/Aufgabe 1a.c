//===========================================================
// Aufgabe 1a: UART6 TX Configuration
// Requirements: ASCII character 'X', 9600 bps, 7E1 format (7 data bits, Even Parity, 1 Stop bit)
//               Transmission via Port PP1 (U6TX)
//===========================================================

// #include "inc/tm4c1294ncpdt.h" // Header of the
// controller type
#include "aufgaben.h"
#include "tm4c1294ncpdt.h"
#include <stdint.h>

#define IDLETIME 1000 // waiting time between transmissions
#define SYSTEM_CLOCK 16000000U // System clock in Hz (assumption: 16 MHz without PLL)

static volatile uint32_t wait_counter_1a = 0;

/**
 * @brief Configures Port P for UART6 TX functionality.
 *
 * Activates the clock for Port P and configures Pin PP1 (Port P, Pin 1)
 * as an alternate function U6TX. Pin PP1 is configured as a digital output
 * with the alternate function for UART6 Transmit enabled.
 */
void config_port_1a(void) {
	// initialize Port P is Bit 13
	SYSCTL_RCGCGPIO_R |= 0x02000; // switch on clock for Port P
	while ((SYSCTL_PRGPIO_R & 0x02000) == 0) {
	}; // Wait until bit 13 of Port P is set. Bit-shift //  while((SYSCTL_PRGPIO_R
		 // & (1 << 13)) == 0) {};
	// wait_counter_1a++;                  // delay for stable clock

	// Configure Pin PP1 (Port P, Pin 1)
	GPIO_PORTP_DEN_R |= 0x2; // enable digital pin function for PP1 |= (1U << 1);
	GPIO_PORTP_DIR_R |= 0x2; // set PP1 as output |= (1U << 1);
	GPIO_PORTP_AFSEL_R |= 0x2; // switch to alternate pin function PP1
	// GPIO_PORTP_PCTL_R |= 0x10; 		!! Dont Use it	// select
	// alternate pin function PP1->U6Tx First clear the old bits for PMC1 (mask
	// 0xFFFFFF0F) and then set the new value.
	GPIO_PORTP_PCTL_R =
			(GPIO_PORTP_PCTL_R & 0xFFFFFF0F) |
			0x00000010; // PCTL für PP1 auf U6Tx 0xFFFFFF0F) | (0x1U << 4);
}

/**
 * @brief Configures UART6 for serial communication.
 *
 * @param baudrate The desired baud rate for UART communication (e.g., 115200).
 *                 Configured for 7E1 format (7 data bits, Even Parity, 1 Stop bit).
 *                 FIFOs are disabled according to the current LCRH setting (0x46).
 */
void config_uart_1a(uint32_t baudrate) {
	// UART6 ist Bit 6 im RCGCUART Register
	SYSCTL_RCGCUART_R |= 0x40; // |= 0x00000040 // switch on clock |= // (1U <<
														 // 6); // Entspricht 0x00000040
	while ((SYSCTL_PRGPIO_R & 0x02000) == 0) {
	}; // Wait until bit 13 of Port P is set. Bit-shift //  while((SYSCTL_PRGPIO_R
		 // & (1 << 13)) == 0) {};
	UART6_CTL_R &= ~0x01; // disable UART6 for config

	// Baudrate konfigurieren
	UART6_IBRD_R = SYSTEM_CLOCK /
								 (16 * baudrate); // set DIVINT of BRD floor(16 MHz/16*9600 bps)
	UART6_FBRD_R = (SYSTEM_CLOCK / (16 * baudrate)) *
								 64; // set DIVFRAC of BRD remaining fraction divider
	
    // Configure serial parameters: 7 data bits, Even Parity, 1 Stop bit (7E1), FIFO disabled
    // UART6_LCRH_R:
    // WLEN (Bits 6:5) = 0b10 (7 data bits)
    // FEN  (Bit 4)    = 0 (FIFO disabled)
    // STP2 (Bit 3)    = 0 (1 Stop bit)
    // EPS  (Bit 2)    = 1 (Even Parity)
    // PEN  (Bit 1)    = 1 (Parity Enable)
    // BRK  (Bit 0)    = 0 (No Break)
    // Value: (0x2 << 5) | (1 << 2) | (1 << 1) = 0x40 | 0x04 | 0x02 = 0x46
    UART6_LCRH_R = 0x00000046;
    // Enable UART6 and Transmit
    // UART6_CTL_R:
    // TXE (Bit 8)    = 1 (Transmit Enable)
    // UARTEN (Bit 0) = 1 (UART Enable)
    // Value: (1 << 8) | (1 << 0) = 0x100 | 0x01 = 0x101
    UART6_CTL_R |= 0x00000101;
}

/**
 * @brief Creates a simple delay using a counting loop.
 *
 * The exact duration depends on the clock frequency and compiler optimizations.
 * 'volatile' is used for the loop.
 */
void idle_1a() { // simple wait for idle_1a state
    volatile int i; // volatile to prevent optimization
	for (i = IDLETIME; i > 0; i--)
		; // count down loop for waiting
}

/**
 * @brief Initializes the necessary hardware (Port P)
 *        UART configuration is done in `execute_aufgabe_1a`.
 */
void init_aufgabe_1a(void) {
    config_port_1a();
}

/**
 * @brief Sends a single character via UART6 continuously.
 *
 * Configures UART6 on the first execution or if the baud rate changes.
 * Waits until the transmit FIFO has space, sends the character, and then waits for a defined time.
 *
 * @param zeichen The character to send.
 * @param baudrate The baud rate to use.
 */
void execute_aufgabe_1a(char zeichen, uint32_t baudrate) {
	// Static variable to ensure that the UART is configured only once
	static uint32_t last_baudrate = 0;

	if (baudrate != last_baudrate) {
		config_uart_1a(baudrate);
		last_baudrate = baudrate;
	}
	while ((UART6_FR_R & 0x20) != 0); // Wait until the transmit FIFO is not full (TXFF flag, Bit 5 in UARTFR register)
	UART6_DR_R = zeichen; // Write character to data register to send
	idle_1a();
}

/**
 * @brief Repeatedly sends a character via UART6 at the specified baud rate.
 *        This function contains an infinite loop.
 *
 * @param zeichen The character to send.
 * @param baudrate The baudrate for UART transmission.
 */
void run_aufgabe_1a(char zeichen, uint32_t baudrate) {
	config_port_1a();         // configuration of Port P
	config_uart_1a(baudrate); // set UART parameters (baudrate)
	while (1) {
		while ((UART6_FR_R & 0x20) != 0); // till transmit FIFO not full
		UART6_DR_R = zeichen; // send the character
		idle_1a();            // idle time
	}
}
