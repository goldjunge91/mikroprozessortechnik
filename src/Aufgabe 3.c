//===========================================================
// Aufgabe 3: UART6 Rx/Tx - Receive and process string from PC
// Requirements: UART6 Rx (PP0) / Tx (PP1), prompt "\r\n>",
//               store input in a buffer (MAXSIZE), print on Enter/EOT.
//===========================================================
#include "aufgaben.h"
#include "tm4c1294ncpdt.h"
#include <stdint.h> // Header with types for the registers
#include <stdio.h>  // For printf

#define IDLETIME 1000 // Waiting time between transmissions
#define MAXSIZE 20 // Max size of Array (Requirement ANF4: 10-50), I chose 20.
#define SYSTEM_CLOCK                                                           \
  16000000 // System clock in Hz (I'm assuming 16 MHz without PLL)

// This counter was likely intended for simple delays.
// It's better to wait for the peripheral ready flags.
static volatile uint32_t wait_counter_3 = 0;

/**
 * @brief Configures Port P for UART6 Rx (PP0) and Tx (PP1) functionality.
 *
 * Activate the clock for Port P first.
 * Then, I'll set up Pin PP0 as U6Rx (input) and PP1 as U6Tx (output).
 * Both need their digital and alternate functions enabled.
 * The PCTL register needs to be set correctly for both pins.
 */
void config_port_3() {
  // initialize Port P
  SYSCTL_RCGCGPIO_R |= 0x02000; // switch on clock for Port P
  //   wait_counter_3++;             // delay for stable clock
  while ((SYSCTL_PRGPIO_R & 0x02000) == 0) {
  }; // Wait until bit 13 of Port P (PRGPIO) is set. Bit-shift //
     // while((SYSCTL_PRGPIO_R & (1U << 13)) == 0) {};

  // initialize Port P
  GPIO_PORTP_DEN_R |= 0x03;  // enable digital pin function for PP1 & PP0
  GPIO_PORTP_DIR_R |= 0x02;  // set PP1 to output
  GPIO_PORTP_DIR_R &= ~0x01; // set PP0 to input

  GPIO_PORTP_AFSEL_R |= 0x03; // switch to alternate pin function PP1 & PP0
  //   GPIO_PORTP_PCTL_R |= 0x10;  // select alternate pin function PP1->U6Tx
  //   GPIO_PORTP_PCTL_R |= 0x01;  // select alternate pin function PP0->U6Rx
  GPIO_PORTP_PCTL_R = (GPIO_PORTP_PCTL_R & 0xFFFFFF00) | 0x00000011;
}

void config_uart_3(uint32_t baudrate, uint32_t lcrh_setting) {
  // initialize UART6
  SYSCTL_RCGCUART_R |= 0x40; // switch on clock for UART6
  wait_counter_3++;          // delay for stable clock
  UART6_CTL_R &= ~0x01;      // disable UART6 for config
  // initialize bitrate of 115200 bit per second
  UART6_IBRD_R = SYSTEM_CLOCK /
                 (16 * baudrate); // set DIVINT of BRD floor(16 MHz/16*9600 bps)
//   UART6_FBRD_R = 44; // set DIVFRAC of BRD remaining fraction divider
  UART6_FBRD_R = (SYSTEM_CLOCK / (16 * baudrate)) *
                 64; // set DIVFRAC of BRD remaining fraction divider
  UART6_LCRH_R = lcrh_setting; // Parameter wird verwendet
  //   UART6_LCRH_R = 0x00000060;   // serial format 8N1

  // Enable UART, Transmit Enable (TXE), and Receive Enable (RXE)
  // UARTEN (bit 0), TXE (bit 8), RXE (bit 9)
  UART6_CTL_R |= 0x0301; // (1<<9) | (1<<8) | (1<<0)
}

void idle_3() {   // simple wait for idle_3 state
  volatile int i; // 'volatile' to prevent compiler optimization
  for (i = IDLETIME; i > 0; i--)
    ; // count down loop for waiting
}

void send_3(char c) {
  while ((UART6_FR_R & 0x20) != 0)
    ;             // till transmit FIFO not full
  UART6_DR_R = c; // sendet Zeichen
  idle_3();       // idle_3 time
}

void run_aufgabe_3(char zeichen, uint32_t baudrate) {
  char buffer[MAXSIZE]; // ANF3 Um aus den Daten einen Array zuerstellen
  char c;               // Um Daten zu empfangen
  int i;
  config_port_3(); // configuration of Port P
  config_uart_3(baudrate, 0x00000060);
  while (1) {
    // ANF1 und ANF7 neuer Prompt
    send_3(0x0D);             // send_3 the character '\r'
    send_3(0x0A);             // send_3 the character '\n'
    send_3('>');              // send_3 the character '>'
    i = 0;                    // Buffer-Index zuruecksetzen
    while (i < MAXSIZE - 1) { // ANF5 minus 1 fuer Platz fuer 0x00
      while (UART6_FR_R & 0x10)
        ;             // ANF2 es wird auf Daten gewartet
      c = UART6_DR_R; // Daten werden Abgelesen um auf EOT zu prüfen
      if (c == 0x04)
        break; // EOT im ASCII Code 0x04 wird erkannt und die Schleife beendet
               // if (i == MAXSIZE-1) break;
      if (c == 0x0D)
        break;       // ANF5 0x0D
      buffer[i] = c; // Zeichen speichern
      i++;
    }
    // buffer[i] = 0x00;               // String beenden
    // printf("Empfangene Nachricht: %s\n", buffer); // ANF6
    buffer[i] = '\0'; // Null-terminate the string (ANF5)
    // Print the received string to the debug console (ANF6)
    printf("Received string: %s\n",
           buffer); // The loop then restarts, fulfilling ANF7.
  }
}

/**
 * @brief Initialization function for Aufgabe 3 (called by task switcher).
 *
 * I just need to configure the port here. UART will be configured
 * in the execute function, as baudrate/format might change.
 */
void init_aufgabe_3(void) { config_port_3(); }

/**
 * @brief Execute function for Aufgabe 3 (called repeatedly by task switcher).
 *
 * @param character_to_send Not directly used for sending, as this task focuses
 * on receiving.
 * @param baudrate The baud rate for UART communication.
 * @param lcrh_setting The LCRH register setting for UART format.
 *
 * This function handles UART (re)configuration, sends prompts,
 * receives characters, echoes them, buffers them, and processes
 * the input upon receiving Enter or EOT, fulfilling ANF1-ANF7.
 */
void execute_aufgabe_3(char zeichen, uint32_t baudrate, uint32_t lcrh_setting) {
  // Statische Variablen, um den Zustand des Buffers zwischen den Aufrufen zu
  // speichern
  static char buffer[MAXSIZE];
  static int i = 0; // Current index in the buffer
  static uint32_t last_baudrate = 0;
  static uint32_t last_lcrh = 0;
  static uint8_t is_initialized = 0;
  char c;

  // Configure UART if parameters have changed or on the very first call
  if (baudrate != last_baudrate || lcrh_setting != last_lcrh) {
    config_uart_3(baudrate, lcrh_setting);
    last_baudrate = baudrate;
    last_lcrh = lcrh_setting;
    is_initialized = 0; // Erzwingt neuen Prompt nach Neukonfiguration
  }
  // Beim ersten Durchlauf nach der Initialisierung einen Prompt senden
  if (!is_initialized) {
    send_3(0x0D); // send_3 the character ('\r'); // send_3('\r');
    send_3(0x0A); // send_3 the character ('\n'); // send_3('\n');
    send_3('>');  // send_3 the character ('>');  // send_3('>');
    is_initialized = 1;
  }

  // Check if a character has been received (non-blocking) (ANF2)
  // RXFE (bit 4 of UARTFR) is 0 if data is available.
  if ((UART6_FR_R & 0x10) == 0) {
    c = UART6_DR_R;

    // Echo an den Benutzer
    send_3(c);

    // if (c == '\r' || c == 0x04) { // Eingabe beendet mit Enter oder EOT
    if (c == 0x04 || c == 0x0D) {
      buffer[i] = '\0';
      printf("Aufgabe 3 empfing: %s\n", buffer); // Ausgabe auf Debug-Konsole
      i = 0;                                     // Buffer zurücksetzen
      send_3(0x0D);                              // send_3 the character ('\r');
      send_3(0x0A);                              // send_3 the character ('\n');
      send_3('>');                               // send_3 the character ('>');
    } else if (i < MAXSIZE - 1) {
      buffer[i++] = c; // Zeichen im Puffer speichern
    }
  }
}
