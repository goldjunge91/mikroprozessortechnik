#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>

#define MAXSIZE 10

/**
 * @brief Konfiguriert Port P für die Verwendung mit UART6.
 *
 * PP1 wird als UART6_TX (Ausgang) und PP0 als UART6_RX (Eingang) konfiguriert.
 */
void config_port_aufgabe3(void) {
    // 1. Takt für Port P aktivieren.
    SYSCTL_RCGCGPIO_R |= 0x02000;

    // 2. Warten, bis der Takt für Port P stabil ist.
    while((SYSCTL_PRGPIO_R & 0x02000) == 0) {};

    // 3. Alternate Function für PP0 (Rx) und PP1 (Tx) aktivieren.
    GPIO_PORTP_AFSEL_R |= 0x03;

    // 4. Pins PP0 und PP1 sicher für die UART-Funktion (Wert 1) konfigurieren.
    GPIO_PORTP_PCTL_R = (GPIO_PORTP_PCTL_R & 0xFFFFFF00) | 0x00000011;

    // 5. Digitale Funktion für PP0 und PP1 aktivieren.
    GPIO_PORTP_DEN_R |= 0x03;

    // 6. Pin-Richtung konfigurieren: PP1 (Tx) als Ausgang, PP0 (Rx) als Eingang.
    GPIO_PORTP_DIR_R |= 0x02;
    GPIO_PORTP_DIR_R &= ~0x01;
}

/**
 * @brief Konfiguriert UART6 für die serielle Kommunikation.
 *
 * Baudrate: 115200 bps, Datenformat: 8N1
 */
void config_uart_aufgabe3(void) {
    // 1. Takt für UART6 aktivieren.
    SYSCTL_RCGCUART_R |= 0x40;

    // 2. Warten, bis der Takt für UART6 stabil ist.
    while((SYSCTL_PRUART_R & 0x40) == 0) {};

    // 3. UART6 vor der Konfiguration deaktivieren.
    UART6_CTL_R &= ~0x01;

    // 4. Baudrate auf 115200 bps einstellen.
    UART6_IBRD_R = 8;
    UART6_FBRD_R = 44;

    // 5. Serielles Format konfigurieren: 8 Datenbits, FIFO aktiviert.
    // Der Wert 0x70 aktiviert den FIFO-Puffer, 0x60 würde ihn deaktivieren.
    // Da wir FIFO-Flags (TXFF, RXFE) verwenden, sollten wir ihn aktivieren.
    UART6_LCRH_R = 0x00000070;

    // 6. UART6, Sender (TXE) und Empfänger (RXE) aktivieren.
    UART6_CTL_R |= 0x0301;
}

/**
 * @brief Sendet ein einzelnes Zeichen über UART6.
 */
void sende_zeichen_aufgabe3(char zeichen) {
    // Warten, solange der Sende-Puffer (FIFO) voll ist.
    while((UART6_FR_R & 0x20) != 0);
    // Zeichen in das Datenregister schreiben.
    UART6_DR_R = zeichen;
}

/**
 * @brief Hauptfunktion für Aufgabe 3.
 */
  
void run_aufgabe_3(void) {
    char zeichen;
    char buffer[MAXSIZE];
    int i;

    config_port_aufgabe3();
    config_uart_aufgabe3();

    while(1) {
        // Anforderung 1 & 7: Prompt senden.
        sende_zeichen_aufgabe3('\r');
        sende_zeichen_aufgabe3('\n');
        sende_zeichen_aufgabe3('>');

        // ===================================================================
        // === KORREKTUR: Empfangspuffer leeren ("flushen")                ===
        // ===================================================================
        // Diese Schleife liest alle "übrig gebliebenen" Zeichen aus dem
        // Puffer, um sicherzustellen, dass er leer ist, bevor wir auf
        // eine neue Eingabe warten.
        while((UART6_FR_R & 0x10) == 0) {
            zeichen = UART6_DR_R; // Zeichen lesen und verwerfen
        }
        // ===================================================================

        i = 0;
        while (i < MAXSIZE - 1) {
            // Warten, bis ein Zeichen empfangen wurde.
            // Diese Schleife wird jetzt korrekt anhalten und warten.
            while((UART6_FR_R & 0x10) != 0);
            zeichen = UART6_DR_R;

            // Anforderung 5: Bei 'Enter' oder EOT die Schleife verlassen.
            if (zeichen == '\r' || zeichen == 0x04) {
                break;
            }
            buffer[i++] = zeichen;
        }

        // Anforderung 5: String mit Null-Terminator abschließen.
        buffer[i] = '\0';

        // Anforderung 6: Empfangenen String auf der Konsole ausgeben.
        printf("Empfangene Nachricht: %s\n", buffer);
    }
}

// // Makro, um die maximale Größe des Empfangspuffers festzulegen.
// // Anforderung 4: MAXSIZE als Makro definieren.
// #define MAXSIZE 10
// static int wait_counter_3 = 0;

// /**
//  * @brief Konfiguriert Port P für die Verwendung mit UART6.
//  *
//  * Anforderung 5.3: Sender (Tx) und Empfänger (Rx) mit Port P verbinden.
//  * PP1 wird als UART6_TX (Ausgang) und PP0 als UART6_RX (Eingang) konfiguriert.
//  */
// void config_port_aufgabe3(void) {
//     // 1. Takt für Port P aktivieren.
//     // SYSCTL_RCGCGPIO_R |= (1 << 13); Methode Mit Bit-Shift
//     SYSCTL_RCGCGPIO_R |= 0x02000;   // switch on clock for Port P
//     // wait_counter_3++; // Es gibt keine Garantie, dass die Verzögerung durch wait_counter_3++ lang genug ist.
//     // Warten Takt für Port P stabil ist. robust und ordentliche variante kein fusch 
//     while((SYSCTL_PRGPIO_R & 0x02000) == 0) {}; // Sende anfrage an Port P zu aktivieren.

//     // 3. Alternate Function für PP0 (Rx) und PP1 (Tx) aktivieren.
//     // GPIO_PORTP_AFSEL_R |= 0x2;      // switch to alternate pin function PP1
//     // GPIO_PORTP_AFSEL_R |= 0x1;      // switch to alternate pin function PP0
//     GPIO_PORTP_AFSEL_R |= 0x03;        // switch to alternate pin function PP0 & PP1

//     // 4. Pins PP0 und PP1 für die UART-Funktion konfigurieren.
//     GPIO_PORTP_PCTL_R |= 0x1;       // select alternate pin function PP1->U6Rx???
//     GPIO_PORTP_PCTL_R |= 0x10;      // select alternate pin function PP1->U6Tx???
//     GPIO_PORTP_PCTL_R = (GPIO_PORTP_PCTL_R & 0xFFFFFF00) | 0x00000011; // erklärugn`?

//     // 5. Digitale Funktion für PP0 und PP1 aktivieren.
//     GPIO_PORTP_DEN_R |= 0x03;

//     // 6. Pin-Richtung konfigurieren: PP1 (Tx) als Ausgang, PP0 (Rx) als Eingang.
//     GPIO_PORTP_DIR_R |= 0x02;   // PP1 als Ausgang
//     GPIO_PORTP_DIR_R &= ~0x01;  // PP0 als Eingang
// }

// /**
//  * @brief Konfiguriert UART6 für die serielle Kommunikation.
//  *
//  * Baudrate: 115200 bps
//  * Datenformat: 8N1 (8 Datenbits, keine Parität, 1 Stoppbit)
//  */
// void config_uart_aufgabe3(void) {
//     // 1. Takt für UART6 aktivieren.
//     // SYSCTL_RCGCUART_R |= 0x40;      // switch on clock for UART6
//     SYSCTL_RCGCUART_R |= (1 << 6);
//     wait_counter_3++;                           // delay for stable clock
//     // 2. Warten, bis der Takt für UART6 stabil ist.
//     while((SYSCTL_PRUART_R & (1 << 6)) == 0) {};
//     // 3. UART6 vor der Konfiguration deaktivieren.
//     UART6_CTL_R &= ~0x01;
//     // 4. Baudrate auf 115200 bps einstellen.
//     UART6_IBRD_R = 8;
//     UART6_FBRD_R = 44;
//     // 5. Serielles Format konfigurieren: 8 Datenbits, FIFO aktiviert.
//     UART6_LCRH_R = 0x00000060;      // serial format 8N1
//     // UART6_LCRH_R = 0x00000070;
//     // 6. UART6, Sender (TXE) und Empfänger (RXE) aktivieren.
//     UART6_CTL_R |= 0x0301;
// }



// /**
//  * @brief Sendet ein einzelnes Zeichen über UART6.
//  *
//  * Die Funktion wartet aktiv (Polling), bis im Sendepuffer Platz ist.
//  * @param zeichen Das zu sendende Zeichen.
//  */
// void sende_zeichen_aufgabe3(char zeichen) {
//     // Warten, solange der Sende-Puffer (FIFO) voll ist.
//     while((UART6_FR_R & 0x20) != 0);
//     // Zeichen in das Datenregister schreiben.
//     UART6_DR_R = zeichen;
// }

// /**
//  * @brief Hauptfunktion für Aufgabe 3.
//  */
// void run_aufgabe_3(void) {
//     char zeichen;
//     char buffer[MAXSIZE];
//     int i;

//     config_port_aufgabe3();
//     config_uart_aufgabe3();

//     while(1) {
//         // Anforderung 1 & 7: Prompt senden.
//         sende_zeichen_aufgabe3('\r');
//         sende_zeichen_aufgabe3('\n');
//         sende_zeichen_aufgabe3('>');

//         i = 0;
//         while (i < MAXSIZE - 1) {
//             // Warten, bis ein Zeichen empfangen wurde.
//             while((UART6_FR_R & 0x10) != 0);
//             zeichen = UART6_DR_R;
//             // Anforderung 5: Bei 'Enter' die Schleife verlassen.
//             if (zeichen == '\r' || zeichen == 0x04) {
//                 break;
//             }
//             buffer[i++] = zeichen;
//         }
//         // Anforderung 5: String mit Null-Terminator abschließen.
//         buffer[i] = '\0';
//         // Anforderung 6: Empfangenen String auf der Konsole ausgeben.
//         printf("Empfangene Nachricht: %s\n", buffer);
//     }
// }


// //===========================================================
// //  Example Program for the TIVA TM4C1294XL Evaluation Board
// //  With this file the LEDs D0 to D3 on the eval board will
// //  blink each one after another. To control the LEDs the
// //  Ports N and F with Pins PN0, PN1, PF0 and PF4 have to be
// //  enabled and set as digital output.
// //===========================================================
// // Include the Header File for controller tm4c1294ncpdt

// #include "inc/tm4c1294ncpdt.h"
// #include <stdint.h>

// // Anforderung 4: MAXSIZE definieren
// #define MAXSIZE 10 
// #define IDLETIME 1000
// static int wait_counter_3 = 0;
// static int a = 0;

// void config_port_aufgabe_3(void){
//     // initialize Port P
//     SYSCTL_RCGCGPIO_R |= 0x02000;   // switch on clock for Port P
//     // wait_counter_3++; // Es gibt keine Garantie, dass die Verzögerung durch wait_counter_3++ lang genug ist.
//     // Warten Takt für Port P stabil ist. robust und ordentliche variante kein fusch 
//     while((SYSCTL_PRGPIO_R & 0x02000) == 0) {}; // Sende anfrage an Port P zu aktivieren.
//     // initialize Port P U6Tx
//     GPIO_PORTP_DEN_R |= 0x2;        // enable digital pin function for PP1
//     GPIO_PORTP_DIR_R |= 0x2;        // set PP1 to output
//     GPIO_PORTP_AFSEL_R |= 0x2;      // switch to alternate pin function PP1
//     GPIO_PORTP_PCTL_R |= 0x10;      // select alternate pin function PP1->U6Tx
//     // initialize Port P U6Rx
//     GPIO_PORTP_DEN_R |= 0x1;        // enable digital pin function for PP0
//     GPIO_PORTP_DIR_R |= ~0x1;       // set PP0 to input
//     GPIO_PORTP_AFSEL_R |= 0x1;      // switch to alternate pin function PP0
//     GPIO_PORTP_PCTL_R |= 0x1;       // select alternate pin function PP1->U6Rx
//     }
// void config_uart_aufgabe_3(void){
//     // initialize UART6
//     SYSCTL_RCGCUART_R |= 0x40;      // switch on clock for UART6
//     wait_counter_3++;                           // delay for stable clock
//     UART6_CTL_R &= ~0x01;           // disable UART6 for config
//                                     // initialize bitrate of 115200 bit per second
//     UART6_IBRD_R = 8;               // set DIVINT of BRD floor(16 MHz/16*115200 bps)
//     UART6_FBRD_R = 44;              // set DIVFRAC of BRD remaining fraction divider
//     UART6_LCRH_R = 0x00000060;      // serial format 8N1
//     UART6_CTL_R |= 0x0301;          // UART transmit on/reciever on and UART enable
//     }

    
// void sende_zeichen_aufgabe_3(char zeichen) {
//     // Warten, solange der Sende-Puffer (FIFO) voll ist.
//     while((UART6_FR_R & 0x20) != 0);
//     // Zeichen in das Datenregister schreiben.
//     UART6_DR_R = zeichen;
// }

// void idle_aufgabe_3() {                       // simple wait for idle_aufgabe_3 state
//     int i = 0;
//     for (i=IDLETIME;i>0;i--);           // count down loop for waiting
// }

// void run_aufgabe_3(void){
//     char zeichen,                               // Char c um Datenarray zu empfangen
//     char buffer[MAXSIZE];                       //buffer um Array zu erstellen
//     int i = 0;                                  //mit Maximallänger
//     config_port_aufgabe_3();                    // configuration of Port P
//     config_port_aufgabe_3();                    // configuration of UART6
//     while(1){
//             while((UART6_FR_R & 0x20) !=0);     // till transmit FIFO not full
//             UART6_DR_R = '\r';                  // send the character '\r'
//             idle_aufgabe_3();                              // idle_aufgabe_3 time
//             UART6_DR_R = '\n';                   // send the character '\n'
//             idle_aufgabe_3();
//             UART6_DR_R = '>';                   // send the character '>'
//             idle_aufgabe_3();
//             while(i < MAXSIZE){

//                 while(UART6_FR_R & 0x10);       // es wird auf Daten gewartet
//                 c = UART6_DR_R;                 // Daten werden Abgelesen um auf EOT zu prüfen
//                 buffer[i] = c;                  // Ausgabe c
//                 if (c == 0x04) break;           // EOT im ASCII Code 0x04 wird erkannt und die Schleife beendet
//                 if (i == MAXSIZE-1) break;      // Anforderung 5 MAXSIZE -1
//                 if (c == 0x0D) break;           // Anforderung 5 0x0D
//                 i++;
//             }
//             buffer[i] = 0x00;                   // letzter slot des Arrays wir mit "\0" beschrieben
//             i = 0;
//             printf("\nGesendete Nachricht: %s\n",buffer);
//     }
// }

