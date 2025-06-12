//===========================================================
// Test program UART6 TX 8/N/1 @ 115200 bit via Port PP1
// LTL 17.5.2020 / mod. V0.2 - V0.4 RMS 1.6.2023
//==========================================================
// #include "inc/tm4c1294ncpdt.h" 			// Header of the
// controller type
#include "aufgaben.h"
#include "tm4c1294ncpdt.h"
#include <stdint.h>

#define IDLETIME 1000 // waiting time between transmissions
#define SYSTEM_CLOCK                                                           \
	16000000U // System clock in Hz (assumption: 16 MHz without PLL)
static volatile uint32_t wait_counter_1a = 0;

/**
 * @brief Configures Port P for UART6 TX functionality.
 *
 * Activates the clock for Port P and configures Pin PP1 as an alternate
 * function U6TX. Pin PP1 is configured as a digital output with the alternate
 * function for UART6 Transmit enabled.
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
// void config_uart_1a(int bps){
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
	// UART6_LCRH_R = 0x00000046; 			// serial format 7E1
	// UART6_CTL_R |= 0x0101; 			// UART transmit on and UART
	// enable Serielle Parameter: 7 Datenbits, Even Parity, 1 Stopbit (7E1), FIFO
	// deaktiviert
	// WLEN=0b10 (7 bits, Bits 6:5), PEN=1 (Bit 1), EPS=1 (Bit 2), FEN=0 (Bit 4)
	UART6_LCRH_R = 0x00000046; // Entspricht: (0x2<<5) | (1<<2) | (1<<1)

	// UART6 aktivieren und Senden aktivieren
	// UARTEN = Bit 0, TXE = Bit 8
	UART6_CTL_R |= 0x00000101; // (1<<8) | (1<<0)
}

void idle_1a() { // simple wait for idle_1a state
	int i;
	for (i = IDLETIME; i > 0; i--)
		; // count down loop for waiting
}

void run_aufgabe_1a(char zeichen, uint32_t baudrate) {
	config_port_1a();         // configuration of Port P
	config_uart_1a(baudrate); // The baud rate is passed on here
	while (1) {
		while ((UART6_FR_R & 0x20) != 0); // till transmit FIFO not full
		UART6_DR_R = zeichen; // send the character
		idle_1a();            // idle time
	}
}

void init_aufgabe_1a(void) { config_port_1a(); }

void execute_aufgabe_1a(char zeichen, uint32_t baudrate) {
	// Static variable to ensure that the UART is configured only once
	static uint32_t last_baudrate = 0;
	if (baudrate != last_baudrate) {
		config_uart_1a(baudrate);
		last_baudrate = baudrate;
	}
	while ((UART6_FR_R & 0x20) != 0);
	UART6_DR_R = zeichen; // Send a character
	idle_1a();
}
