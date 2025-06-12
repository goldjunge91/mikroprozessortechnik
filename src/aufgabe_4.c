//############################################################################
//###                                                                      ###
//###      AUFGABE 4: LED Steuerung über UART                              ###
//###                                                                      ###
//############################################################################
// #include "inc/tm4c1294ncpdt.h" 			// Header of the controller type
#include "tm4c1294ncpdt.h"
#include <stdio.h>
#include <stdint.h>

#define IDLETIME 1000
#define MAXSIZE 10
#define PORTM 0x00000800
// int wait_counter_4 = 0;
volatile int wait_counter_4 = 0; // 'volatile' verhindert, dass der Compiler die Warteschleife optimiert.
// void config_port_aufgabe_4(void) {
//     // Clock für Port P (UART) und Port M (LEDs)
//     SYSCTL_RCGCGPIO_R |= 0x02000;   // switch on clock for Port P
//     SYSCTL_RCGCGPIO_R |= 0x00000800;   // switch on clock for Port M
//     wait_counter_4++;
//     // Port P für UART6 (Rx/Tx) konfigurieren
//     GPIO_PORTP_DEN_R |= 0x03;       // enable digital pin function for PP1 & PP0
//     GPIO_PORTP_DIR_R |= 0x02;       // set PP1 to output
//     GPIO_PORTP_DIR_R &= ~0x01;      // set PP0 to input
//     GPIO_PORTP_AFSEL_R |= 0x03;     // switch to alternate pin function PP1 & PP0
//     GPIO_PORTP_PCTL_R = (GPIO_PORTP_PCTL_R & 0xFFFFFF00) | 0x00000011; // PCTL für U6Rx/U6Tx
// }
void config_port_aufgabe_4(void) {
    // Clock für Port P (UART), Port N (LEDs vom Tiva), Port F (echte LEDs) und Port M (Simulation)
    SYSCTL_RCGCGPIO_R |= 0x02000; // switch on clock for Port P
    SYSCTL_RCGCGPIO_R |= 0x00000800;   // switch on clock for Port M
    
    wait_counter_4++;
    
    SYSCTL_RCGCGPIO_R |= 0x2000; // |= (1 << 13); // Takt für Port P (UART) (entspricht 1 << 13)
    SYSCTL_RCGCGPIO_R |= 0x1000; // |= (1 << 12); // Takt für Port N (LEDs D1, D2) (entspricht 1 << 12)
    SYSCTL_RCGCGPIO_R |= 0x0020; // |= (1 << 5);  // Takt für Port F (LEDs D3, D4) (entspricht 1 << 5)
    SYSCTL_RCGCGPIO_R |= 0x0800; // |= (1 << 11); // TODO: Takt für Port M ANFxx (entspricht 1 << 11)

    // Warte auf Taktversorgung für die GPIO-Ports stabil ist,
    // bevor auf deren Register zugegriffen wird.
    while((SYSCTL_PRGPIO_R & 0x1000) == 0) {}; // Korrekte Prüfung: Warte, bis Bit 12 von Port N gesetzt ist. Bit-shift //  while((SYSCTL_PRGPIO_R & (1 << 12)) == 0) {};
    while((SYSCTL_PRGPIO_R & 0x0020) == 0) {};  // Warte auf Port F (LEDs D3, D4) (entspricht 1 << 5)


    // Port P für UART6 (Rx/Tx) konfigurieren
    GPIO_PORTP_DEN_R |= 0x03;       // PP1 & PP0 digital aktivieren
    GPIO_PORTP_DIR_R |= 0x02;       // PP1 (Tx) als Ausgang
    GPIO_PORTP_DIR_R &= ~0x01;      // PP0 (Rx) als Eingang
    GPIO_PORTP_AFSEL_R |= 0x03;     // Alternate Function für PP1 & PP0
    GPIO_PORTP_PCTL_R = (GPIO_PORTP_PCTL_R & 0xFFFFFF00) | 0x00000011;

    // Port N für ECHTE LEDs D1 (PN1) und D2 (PN0) konfigurieren
    GPIO_PORTN_DEN_R |= 0x03;       // PN1 & PN0 digital aktivieren
    GPIO_PORTN_DIR_R |= 0x03;       // PN1 & PN0 als Ausgänge

    // Port F für ECHTE LEDs D3 (PF4) und D4 (PF0) konfigurieren
    GPIO_PORTF_AHB_DEN_R |= 0x11;   // PF4 & PF0 digital aktivieren
    GPIO_PORTF_AHB_DIR_R |= 0x11;   // PF4 & PF0 als Ausgänge
    
    // Port M für die "SIMULATION" laut Anleitung konfigurieren
    GPIO_PORTM_DEN_R |= 0x0F;       // PM0-PM3 digital aktivieren
    GPIO_PORTM_DIR_R |= 0x0F;       // PM0-PM3 als Ausgänge
}

void config_uart_aufgabe_4(uint32_t baudrate, uint32_t lcrh_setting) {
    // #define BPS 115200 // Aufgabe 4 nutzt oft eine höhere Baudrate
    SYSCTL_RCGCUART_R |= 0x40;      // switch on clock for UART6
    wait_counter_4++;                           // delay for stable clock
    while((SYSCTL_PRUART_R & 0x40) == 0) {};
    UART6_CTL_R &= ~0x01;           // disable UART6 for config
    UART6_IBRD_R = 8;               // set DIVINT of BRD floor(16 MHz / (16 * 115200 bps)) = 8
    UART6_FBRD_R = 44;              // set DIVFRAC of BRD = round(0.6805 * 64) = 44
    // UART6_LCRH_R = 0x00000060;      // serial format 8N1
    UART6_LCRH_R = lcrh_setting; // Parameter verwenden
    UART6_CTL_R |= 0x0301;          // UART transmit/receive on and UART enable
}

void send_char4(char zeichen_zum_senden) {
    // Aktives Warten (Polling), solange der Sende-Puffer voll ist.
    // Das TXFF-Flag (Bit 5) im Flag-Register ist 1, solange der Puffer voll ist.
    while ((UART6_FR_R & 0x20) != 0);
    // Wenn die Schleife beendet wurde, ist Platz im Puffer.
    // Schreibe das zu sendende Zeichen in das Datenregister.
    UART6_DR_R = zeichen_zum_senden;
}

// --- Main-Funktion für Aufgabe 4
void aufgabe_4(uint32_t baudrate) {
    char empfange_zeichen;
    char buffer[MAXSIZE];
    int i;
    config_port_aufgabe_4();
    config_uart_aufgabe_4(baudrate, 0x00000060); // Provide baudrate and LCRH setting
    // Initialzustand: Alle LEDs einschalten
    GPIO_PORTM_DATA_R |= 0x0F;      // Schaltet simulierte LEDs an Port M an
    GPIO_PORTN_DATA_R |= 0x03;      // Schaltet echte LEDs D1 (PN1) und D2 (PN0) an
    GPIO_PORTF_AHB_DATA_R |= 0x11;  // Schaltet echte LEDs D3 (PF4) und D4 (PF0) an

    while (1) {                     // Hauptschleife
        // Sendet den Prompt ">" in einer neuen Zeile an das Terminal
        send_char4('\r'); // Zeilenumbruch
        send_char4('\n'); // Neue Zeile
        send_char4('>');  // Sendet das Zeichen '>' 

        i = 0;
        while (i < MAXSIZE - 1) {
            // Wartet solange der Empfangs-Puffer (FIFO) leer ist.
            // Die Maske 0x10 prüft Bit 4. Solange dieses Bit 1 ist (Puffer leer),
            while ((UART6_FR_R & 0x10) != 0); // Warte Loop
            empfange_zeichen = UART6_DR_R; // Das empfangene Zeichen aus dem Datenregister holen
            if (empfange_zeichen == '\r' // Abbruch bei Carriage Return oder End-Of-Transmission (EOT)
                || 
                empfange_zeichen == 0x04) 
                break; 
            buffer[i++] = empfange_zeichen; // Das gültige Zeichen in den Puffer schreiben und den Zähler für die nächste Position erhöhen.
        }
        buffer[i] = '\0'; // Den String mit einem Null-Terminator abschließen. Das ist wichtig.
        printf("Befehl empfangen: %s\n", buffer);
        // Befehl parsen und auf alle relevanten Ports anwenden
        if (buffer[0] == 'l' && buffer[1] == 'e' && buffer[2] == 'd') {
            char operation = buffer[3]; // '+' oder '-'
            char led_num = buffer[4];   // '0' bis '3'

            if (operation == '+') {
                switch (led_num) {
                    case '0':
                        GPIO_PORTF_AHB_DATA_R |= 0x01; // Echte LED D4 (PF0) an
                        GPIO_PORTM_DATA_R     |= 0x01; // Simulation für PM0 an
                        break;
                    case '1':
                        GPIO_PORTF_AHB_DATA_R |= 0x10; // Echte LED D3 (PF4) an
                        GPIO_PORTM_DATA_R     |= 0x02; // Simulation für PM1 an
                        break;
                    case '2':
                        GPIO_PORTN_DATA_R     |= 0x01; // Echte LED D2 (PN0) an
                        GPIO_PORTM_DATA_R     |= 0x04; // Simulation für PM2 an
                        break;
                    case '3':
                        GPIO_PORTN_DATA_R     |= 0x02; // Echte LED D1 (PN1) an
                        GPIO_PORTM_DATA_R     |= 0x08; // Simulation für PM3 an
                        break;
                }
            } else if (operation == '-') {
                switch (led_num) {
                    case '0':
                        GPIO_PORTF_AHB_DATA_R &= ~0x01; // Echte LED D4 (PF0) aus
                        GPIO_PORTM_DATA_R     &= ~0x01; // Simulation für PM0 aus
                        break;
                    case '1':
                        GPIO_PORTF_AHB_DATA_R &= ~0x10; // Echte LED D3 (PF4) aus
                        GPIO_PORTM_DATA_R     &= ~0x02; // Simulation für PM1 aus
                        break;
                    case '2':
                        GPIO_PORTN_DATA_R     &= ~0x01; // Echte LED D2 (PN0) aus
                        GPIO_PORTM_DATA_R     &= ~0x04; // Simulation für PM2 aus
                        break;
                    case '3':
                        GPIO_PORTN_DATA_R     &= ~0x02; // Echte LED D1 (PN1) aus
                        GPIO_PORTM_DATA_R     &= ~0x08; // Simulation für PM3 aus
                        break;
                }
            }
        }
    }
}

void parse_and_execute_led_command(const char* buffer) {
    if (buffer[0] == 'l' && buffer[1] == 'e' && buffer[2] == 'd') {
        char operation = buffer[3];
        char led_num = buffer[4];

         if (operation == '+') {
                switch (led_num) {
                    case '0':
                        GPIO_PORTF_AHB_DATA_R |= 0x01; // Echte LED D4 (PF0) an
                        GPIO_PORTM_DATA_R     |= 0x01; // Simulation für PM0 an
                        break;
                    case '1':
                        GPIO_PORTF_AHB_DATA_R |= 0x10; // Echte LED D3 (PF4) an
                        GPIO_PORTM_DATA_R     |= 0x02; // Simulation für PM1 an
                        break;
                    case '2':
                        GPIO_PORTN_DATA_R     |= 0x01; // Echte LED D2 (PN0) an
                        GPIO_PORTM_DATA_R     |= 0x04; // Simulation für PM2 an
                        break;
                    case '3':
                        GPIO_PORTN_DATA_R     |= 0x02; // Echte LED D1 (PN1) an
                        GPIO_PORTM_DATA_R     |= 0x08; // Simulation für PM3 an
                        break;
                }
            } else if (operation == '-') {
                switch (led_num) {
                    case '0':
                        GPIO_PORTF_AHB_DATA_R &= ~0x01; // Echte LED D4 (PF0) aus
                        GPIO_PORTM_DATA_R     &= ~0x01; // Simulation für PM0 aus
                        break;
                    case '1':
                        GPIO_PORTF_AHB_DATA_R &= ~0x10; // Echte LED D3 (PF4) aus
                        GPIO_PORTM_DATA_R     &= ~0x02; // Simulation für PM1 aus
                        break;
                    case '2':
                        GPIO_PORTN_DATA_R     &= ~0x01; // Echte LED D2 (PN0) aus
                        GPIO_PORTM_DATA_R     &= ~0x04; // Simulation für PM2 aus
                        break;
                    case '3':
                        GPIO_PORTN_DATA_R     &= ~0x02; // Echte LED D1 (PN1) aus
                        GPIO_PORTM_DATA_R     &= ~0x08; // Simulation für PM3 aus
                        break;
                }
            }
    }
}
// init-Funktion
void init_aufgabe_4(void) {
    config_port_aufgabe_4();
    // config_uart_aufgabe_4();
    // Initialzustand: Alle LEDs aus
    GPIO_PORTN_DATA_R &= ~0x03;
    GPIO_PORTF_AHB_DATA_R &= ~0x11;
    GPIO_PORTM_DATA_R &= ~0x0F;
}


// execute-Funktion: Passt jetzt zur Signatur in aufgaben.h
void execute_aufgabe_4(char zeichen, uint32_t baudrate, uint32_t lcrh_setting) {
    static char buffer[MAXSIZE];
    static int i = 0;
    char c;
    static uint32_t last_baudrate = 0;
    static uint32_t last_lcrh = 0;
    static uint8_t is_initialized = 0;
    // UART konfigurieren, falls sich Parameter geändert haben oder beim allerersten Aufruf
    if (baudrate != last_baudrate || lcrh_setting != last_lcrh) {
        config_uart_aufgabe_4(baudrate, lcrh_setting);
        last_baudrate = baudrate;
        last_lcrh = lcrh_setting;
        is_initialized = 0; // Erzwingt neuen Prompt nach Neukonfiguration
    }
    if (!is_initialized) {
        send_char4('\r');
        send_char4('\n');
        send_char4('>');
        is_initialized = 1;
    }

    // Prüfen, ob ein Zeichen im Empfangspuffer ist
    if ((UART6_FR_R & 0x10) == 0) {
        c = UART6_DR_R;
        if (c == '\r' || c == 0x04) {
            buffer[i] = '\0';
            send_char4('\r');
            send_char4('\n');
            parse_and_execute_led_command(buffer);
            i = 0;
            send_char4('>');
        } else if (i < MAXSIZE - 1) {
            send_char4(c); // Echo
            buffer[i++] = c;
        }
    }
}
