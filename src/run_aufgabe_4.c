//############################################################################
//###                                                                      ###
//###      AUFGABE 4: LED Steuerung über UART                              ###
//###                                                                      ###
//############################################################################
#include "inc/tm4c1294ncpdt.h"
#include <stdio.h>
#include <stdint.h>

#define IDLETIME 1000
#define MAXSIZE 10
#define PORTM 0x00000800
// int wait_counter_4 = 0;
volatile int wait_counter_4 = 0; // 'volatile' verhindert, dass der Compiler die Warteschleife optimiert.

// void config_port_aufgabe4(void) {
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
//     // Port M für LEDs (PM0-PM3) konfigurieren
//     GPIO_PORTM_DEN_R |= 0x0F;       // enable digital pin function for PM0-PM3
//     GPIO_PORTM_DIR_R |= 0x0F;       // set PM0-PM3 to output
// }
void config_port_aufgabe4(void) {
    // Clock für Port P (UART), Port N (echte LEDs), Port F (echte LEDs) und Port M (Simulation)
    SYSCTL_RCGCGPIO_R |= (1 << 13); // Takt für Port P (UART)
    SYSCTL_RCGCGPIO_R |= (1 << 12); // Takt für Port N (LEDs D1, D2)
    SYSCTL_RCGCGPIO_R |= (1 << 11); // Takt für Port M (Simulation laut Anleitung)
    SYSCTL_RCGCGPIO_R |= (1 << 5);  // Takt für Port F (LEDs D3, D4)
    
    // Kurze Wartezeit, bis die Clocks der echten LED-Ports stabil sind
    while((SYSCTL_PRGPIO_R & (1 << 12)) == 0) {};
    while((SYSCTL_PRGPIO_R & (1 << 5)) == 0) {};

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

void config_uart_aufgabe4(void) {
    #define BPS 115200 // Aufgabe 4 nutzt oft eine höhere Baudrate
    SYSCTL_RCGCUART_R |= 0x40;      // switch on clock for UART6
    wait_counter_4++;                           // delay for stable clock
    UART6_CTL_R &= ~0x01;           // disable UART6 for config
    UART6_IBRD_R = 8;               // set DIVINT of BRD floor(16 MHz / (16 * 115200 bps)) = 8
    UART6_FBRD_R = 44;              // set DIVFRAC of BRD = round(0.6805 * 64) = 44
    UART6_LCRH_R = 0x00000060;      // serial format 8N1
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
void run_aufgabe_4(void) {
    char c, buffer[MAXSIZE];
    int i;
    // Verwenden Sie die Konfiguration, die Port M, N und F aktiviert!
    config_port_aufgabe4();
    config_uart_aufgabe4();
    // Initialzustand: Alle LEDs (echte und simulierte) einschalten
    GPIO_PORTM_DATA_R |= 0x0F;      // Schaltet simulierte LEDs an Port M an
    GPIO_PORTN_DATA_R |= 0x03;      // Schaltet echte LEDs D1 (PN1) und D2 (PN0) an
    GPIO_PORTF_AHB_DATA_R |= 0x11;  // Schaltet echte LEDs D3 (PF4) und D4 (PF0) an
    // Hauptschleife
    while (1) {
        // Sendet den Prompt ">" in einer neuen Zeile an das Terminal
        send_char4('\r'); // Wagenrücklauf
        send_char4('\n'); // Zeilenvorschub
        send_char4('>');  // Prompt-Zeichen

        i = 0;
        while (i < MAXSIZE - 1) {
            // Warteschleife: Solange der Empfangs-Puffer (FIFO) leer ist, tue nichts.
            // Die Maske 0x10 prüft Bit 4. Solange dieses Bit 1 ist (Puffer leer),
            while ((UART6_FR_R & 0x10) != 0); // Das Programm bleibt hier hängen, bis ein Zeichen vom PC ankommt.
            empfange_zeichen = UART6_DR_R; // Das empfangene Zeichen aus dem Datenregister holen
            if (empfange_zeichen == '\r' || empfange_zeichen == 0x04) break; // Die Schleife wird bei 'Enter' (ASCII '\r') oder bei EOT verlassen.
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
// void run_aufgabe_4(void) {
//     char c, buffer[MAXSIZE];
//     int i;
//     config_port_aufgabe4();
//     config_uart_aufgabe4();
//     // Initialzustand: Alle LEDs aus
//     GPIO_PORTM_DATA_R &= ~0x0F;
//     while (1) {
//         send_char4('\r');
//         send_char4('\n');
//         send_char4('>');
//         i = 0;
//         while (i < MAXSIZE - 1) {
//             while (UART6_FR_R & 0x10); // Warten auf Zeichen
//             c = UART6_DR_R;
//             if (c == '\r' || c == 0x04) break; // Ende bei Enter oder EOT
//             buffer[i++] = c;
//         }
//         buffer[i] = '\0'; // String terminieren
//         // Empfangene Nachricht zur Kontrolle ausgeben
//         printf("Befehl empfangen: %s\n", buffer);
//         // Befehl parsen und ausführen
//         if (buffer[0] == 'l' && buffer[1] == 'e' && buffer[2] == 'd') {
//             char operation = buffer[3]; // '+' oder '-'
//             char led_num = buffer[4];   // '0' bis '3' (oder '1' bis '4' je nach Spec)
//             if (operation == '+') {
//                 switch (led_num) {
//                     case '0': GPIO_PORTM_DATA_R |= 0x01; break; // LED an PM0
//                     case '1': GPIO_PORTM_DATA_R |= 0x02; break; // LED an PM1
//                     case '2': GPIO_PORTM_DATA_R |= 0x04; break; // LED an PM2
//                     case '3': GPIO_PORTM_DATA_R |= 0x08; break; // LED an PM3
//                 }
//             } else if (operation == '-') {
//                 switch (led_num) {
//                     case '0': GPIO_PORTM_DATA_R &= ~0x01; break;
//                     case '1': GPIO_PORTM_DATA_R &= ~0x02; break;
//                     case '2': GPIO_PORTM_DATA_R &= ~0x04; break;
//                     case '3': GPIO_PORTM_DATA_R &= ~0x08; break;
//                 }
//             }
//         }
//     }
// }
