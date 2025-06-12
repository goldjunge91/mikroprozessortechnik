//############################################################################
//###                                                                      ###
//###      AUFGABE 4: LED Steuerung über UART (Optionale Erweiterungen)    ###
//###                                                                      ###
//############################################################################
#include "tm4c1294ncpdt.h"
#include <stdio.h>
#include <stdint.h>

// #define IDLETIME 1000 // Nicht verwendet in diesem Snippet, kann ggf. entfernt werden
#define MAXSIZE 50      // Puffergröße für UART-Eingabe
// #define PORTM 0x00000800 // Nicht direkt als Makro verwendet, Bitmasken direkt genutzt

// Fehlercodes für das Parsen von Befehlen
#define ERROR_NONE 0
#define ERROR_TOO_SHORT 1
#define ERROR_WRONG_PREFIX 2
#define ERROR_INVALID_OPERATOR 3
#define ERROR_INVALID_LED_NUMBER 4
#define ERROR_TOO_LONG 5
#define ERROR_EMPTY_INPUT 6 // Neuer Fehlercode für leere Eingabe nach remove_spaces

// volatile int wait_counter_4 = 0; // Nicht mehr benötigt und entfernt

void config_port_aufgabe_4_optional(void) {
    // Clock für Port P (UART), Port N (LEDs D1, D2), Port F (LEDs D3, D4) und Port M (Simulation)
    SYSCTL_RCGCGPIO_R |= (1U << 13); // Takt für Port P (UART6)
    SYSCTL_RCGCGPIO_R |= (1U << 12); // Takt für Port N (LEDs D1, D2)
    SYSCTL_RCGCGPIO_R |= (1U << 5);  // Takt für Port F (LEDs D3, D4)
    SYSCTL_RCGCGPIO_R |= (1U << 11); // Takt für Port M (Simulation PM0-PM3)

    // Warte, bis die Taktversorgung für die GPIO-Ports stabil ist
    while ((SYSCTL_PRGPIO_R & (1U << 13)) == 0) {}; // Warten auf Port P
    while ((SYSCTL_PRGPIO_R & (1U << 12)) == 0) {}; // Warten auf Port N
    while ((SYSCTL_PRGPIO_R & (1U << 5)) == 0)  {}; // Warten auf Port F
    while ((SYSCTL_PRGPIO_R & (1U << 11)) == 0) {}; // Warten auf Port M

    // Port P für UART6 (Rx PP0 / Tx PP1) konfigurieren
    GPIO_PORTP_DEN_R |= 0x03;       // PP0 & PP1 digital aktivieren
    GPIO_PORTP_DIR_R &= ~0x01;      // PP0 (Rx) als Eingang setzen
    GPIO_PORTP_DIR_R |= 0x02;       // PP1 (Tx) als Ausgang setzen
    GPIO_PORTP_AFSEL_R |= 0x03;     // Alternate Function für PP0 & PP1 aktivieren
    GPIO_PORTP_PCTL_R = (GPIO_PORTP_PCTL_R & 0xFFFFFF00) | 0x00000011; // PCTL auf UART-Funktion setzen (U6Rx, U6Tx)

    // Port N für ECHTE LEDs D1 (PN1) und D2 (PN0) konfigurieren
    GPIO_PORTN_DEN_R |= 0x03;       // PN0 & PN1 digital aktivieren
    GPIO_PORTN_DIR_R |= 0x03;       // PN0 & PN1 als Ausgänge setzen

    // Port F für ECHTE LEDs D3 (PF4) und D4 (PF0) konfigurieren
    // Beachten Sie, dass Port F AHB verwendet, daher _AHB_ Register
    GPIO_PORTF_AHB_DEN_R |= 0x11;   // PF0 & PF4 digital aktivieren
    GPIO_PORTF_AHB_DIR_R |= 0x11;   // PF0 & PF4 als Ausgänge setzen
    
    // Port M für die "SIMULATION" der LEDs PM0-PM3 laut Anleitung konfigurieren
    GPIO_PORTM_DEN_R |= 0x0F;       // PM0-PM3 digital aktivieren
    GPIO_PORTM_DIR_R |= 0x0F;       // PM0-PM3 als Ausgänge setzen
}

void config_uart_aufgabe_4_optional(uint32_t baudrate, uint32_t lcrh_setting) {
    SYSCTL_RCGCUART_R |= (1U << 6);      // Takt für UART6 (Bit 6) einschalten
    while((SYSCTL_PRUART_R & (1U << 6)) == 0) {}; // Warten, bis UART6-Peripherie bereit ist
    
    UART6_CTL_R &= ~UART_CTL_UARTEN; // UART6 für Konfiguration deaktivieren (UART_CTL_UARTEN ist 0x1)

    // Baudrate fest auf 115200 bps eingestellt (entsprechend Task-Switcher Aufruf für Aufgabe 4)
    // Für 16MHz Systemtakt und 115200 Baud:
    // BRD = 16.000.000 / (16 * 115200) = 8.68055...
    // IBRD = 8
    // FBRD = Integer((0.68055...) * 64 + 0.5) = Integer(43.555... + 0.5) = Integer(44.055...) = 44
    UART6_IBRD_R = 8;
    UART6_FBRD_R = 44;
    
    UART6_LCRH_R = lcrh_setting; // LCRH-Parameter verwenden (z.B. 0x60 für 8N1)
    
    // UART aktivieren, Senden (TXE) und Empfangen (RXE) aktivieren
    UART6_CTL_R |= (UART_CTL_UARTEN | UART_CTL_TXE | UART_CTL_RXE); // Entspricht 0x0301
}

void send_char4_optional(char zeichen_zum_senden) {
    // Warten, bis der Sende-FIFO (Transmit FIFO) nicht mehr voll ist.
    // UART_FR_TXFF (Bit 5) ist 1, wenn FIFO voll.
    while ((UART6_FR_R & UART_FR_TXFF) != 0);
    UART6_DR_R = zeichen_zum_senden; // Zeichen in das Datenregister schreiben
}

// Hilfsfunktion zum Entfernen von Leerzeichen aus einem String (in-place)
void remove_spaces(char* str) {
    if (str == NULL) return;
    char* write_ptr = str;
    char* read_ptr = str;

    while (*read_ptr) {
        if (*read_ptr != ' ') {
            *write_ptr = *read_ptr;
            write_ptr++;
        }
        read_ptr++;
    }
    *write_ptr = '\0';
}

// Parst den Befehl und führt ihn aus, gibt einen Fehlercode zurück.
int parse_and_execute_led_command_optional(char* buffer) {
    remove_spaces(buffer); // Leerzeichen zuerst entfernen

    int len = 0;
    while(buffer[len] != '\0') {
        len++;
    }

    if (len == 0) {
        return ERROR_EMPTY_INPUT; // Eingabe war nur Leerzeichen oder leer
    }
    if (len < 5) {
        return ERROR_TOO_SHORT; // "led+0" ist die Mindestlänge
    }
    // Für die aktuelle Anforderung (einzelner Befehl "led+0" ohne Leerzeichen)
    // ist alles > 5 Zeichen ein Fehler.
    // Wenn kombinierte Befehle wie "led+0+1" erlaubt wären, müsste dies angepasst werden.
    if (len > 5) {
        return ERROR_TOO_LONG; // Oder spezifischer: ERROR_INVALID_FORMAT
    }

    if (buffer[0] != 'l' || buffer[1] != 'e' || buffer[2] != 'd') {
        return ERROR_WRONG_PREFIX;
    }

    char operation = buffer[3];
    char led_num_char = buffer[4];

    if (operation != '+' && operation != '-') {
        return ERROR_INVALID_OPERATOR;
    }

    if (led_num_char < '0' || led_num_char > '3') {
        return ERROR_INVALID_LED_NUMBER;
    }

    // Erfolgreich geparst, führe die Aktion aus
    if (operation == '+') {
        switch (led_num_char) {
            case '0': GPIO_PORTF_AHB_DATA_R |= 0x01; GPIO_PORTM_DATA_R |= 0x01; break; // D4 (PF0), PM0
            case '1': GPIO_PORTF_AHB_DATA_R |= 0x10; GPIO_PORTM_DATA_R |= 0x02; break; // D3 (PF4), PM1
            case '2': GPIO_PORTN_DATA_R     |= 0x01; GPIO_PORTM_DATA_R |= 0x04; break; // D2 (PN0), PM2
            case '3': GPIO_PORTN_DATA_R     |= 0x02; GPIO_PORTM_DATA_R |= 0x08; break; // D1 (PN1), PM3
        }
    } else { // operation == '-'
        switch (led_num_char) {
            case '0': GPIO_PORTF_AHB_DATA_R &= ~0x01; GPIO_PORTM_DATA_R &= ~0x01; break;
            case '1': GPIO_PORTF_AHB_DATA_R &= ~0x10; GPIO_PORTM_DATA_R &= ~0x02; break;
            case '2': GPIO_PORTN_DATA_R     &= ~0x01; GPIO_PORTM_DATA_R &= ~0x04; break;
            case '3': GPIO_PORTN_DATA_R     &= ~0x02; GPIO_PORTM_DATA_R &= ~0x08; break;
        }
    }
    return ERROR_NONE; // Kein Fehler
}

// Execute-Funktion, die vom Task-Switcher aufgerufen wird
void execute_aufgabe_4_optional(char zeichen_param, uint32_t baudrate, uint32_t lcrh_setting) {
    // zeichen_param wird in dieser Implementierung nicht direkt verwendet,
    // da die Eingabe über UART erfolgt.
    static char buffer[MAXSIZE];
    static int buffer_index = 0; // Umbenannt von 'i' für mehr Klarheit
    char received_char;
    static uint32_t last_baudrate = 0;
    static uint32_t last_lcrh = 0;
    static uint8_t is_initialized = 0; // Für initialen Prompt

    // UART konfigurieren, falls sich Parameter geändert haben oder beim allerersten Aufruf
    if (baudrate != last_baudrate || lcrh_setting != last_lcrh || !is_initialized) {
        config_port_aufgabe_4_optional(); // Port-Konfiguration sicherstellen
        config_uart_aufgabe_4_optional(baudrate, lcrh_setting);
        last_baudrate = baudrate;
        last_lcrh = lcrh_setting;
        
        if (!is_initialized) {
            send_char4_optional('\r');
            send_char4_optional('\n');
            send_char4_optional('>');
            is_initialized = 1;
            printf("execute_aufgabe_4_optional: Initialized and prompt sent.\n"); // DEBUG
        }
    }

    if ((UART6_FR_R & UART_FR_RXFE) == 0) {
        received_char = UART6_DR_R;
        // printf("Char received: 0x%02X\n", received_char); // SEHR GESPRÄCHIGES DEBUGGING

        if (received_char == '\r' || received_char == 0x04) { // Enter oder EOT (Ctrl-D)
            buffer[buffer_index] = '\0'; 
            
            send_char4_optional('\r'); 
            send_char4_optional('\n');

            // --- DEBUG AUSGABEN HINZUGEFÜGT ---
            send_char4_optional('D');
            send_char4_optional('E');
            send_char4_optional('B');
            send_char4_optional('U');
            send_char4_optional('G');
            send_char4_optional(':');
            send_char4_optional(' ');
            send_char4_optional('E');
            send_char4_optional('n');
            send_char4_optional('t');
            send_char4_optional('e');
            send_char4_optional('r');
            send_char4_optional(' ');
            send_char4_optional('p');
            send_char4_optional('r');
            send_char4_optional('e');
            send_char4_optional('s');
            send_char4_optional('s');
            send_char4_optional('e');
            send_char4_optional('d');
            send_char4_optional('.');
            send_char4_optional('\n');
            
            // printf("DEBUG: Enter pressed. Buffer content before parse: \"%s\"\n", buffer);
            // --- ENDE DEBUG AUSGABEN ---

            int parse_error_code = parse_and_execute_led_command_optional(buffer);
            
            // --- DEBUG AUSGABEN HINZUGEFÜGT ---
            printf("DEBUG: parse_and_execute_led_command_optional returned: %d\n", parse_error_code);
            // --- ENDE DEBUG AUSGABEN ---


            // Anforderung 5 & Optionale Erweiterung: Ausgabe mit printf() und Errorcode
            printf("Befehl verarbeitet: \"%s\", Errorcode: %d\n", buffer, parse_error_code);

            buffer_index = 0; 
            send_char4_optional('>'); 
        } else if (received_char == 0x08 || received_char == 0x7F) { // Backspace (0x08) oder DEL (0x7F)
            if (buffer_index > 0) {
                buffer_index--;
                send_char4_optional(0x08); // Backspace-Zeichen senden
                send_char4_optional(' ');  // Überschreibt das Zeichen im Terminal
                send_char4_optional(0x08); // Cursor wieder zurück
            }
        } else if (buffer_index < MAXSIZE - 1) {
            // Nur druckbare ASCII-Zeichen oder spezifisch erlaubte Zeichen sammeln
            if ((received_char >= ' ' && received_char <= '~')) {
                 send_char4_optional(received_char); // Echo des Zeichens
                 buffer[buffer_index++] = received_char;
            }
        } else {
            // Puffer voll, aber noch kein Enter.
            // Optional: Fehler signalisieren (z.B. Warnton senden) oder Zeichen ignorieren.
            // Für jetzt: Ignorieren, um Überlauf zu verhindern.
            // Man könnte auch den letzten Prompt erneut senden, um einen Fehler anzuzeigen.
        }
    }
}

// init_aufgabe_4_optional - wird vom Task-Switcher einmalig aufgerufen
void init_aufgabe_4_optional(void) {
    // Die Port-Konfiguration wird jetzt in execute_aufgabe_4_optional beim ersten Mal
    // oder bei Parameteränderung aufgerufen, um sicherzustellen, dass sie aktuell ist.
    // Wenn eine einmalige Initialisierung hier gewünscht ist, die nicht von Baudrate etc. abhängt:
    // config_port_aufgabe_4_optional();
    // Initialzustand der LEDs setzen, falls gewünscht:
    // GPIO_PORTM_DATA_R = 0x00; // Alle Simulations-LEDs aus
    // GPIO_PORTN_DATA_R = 0x00; // Alle echten LEDs D1/D2 aus
    // GPIO_PORTF_AHB_DATA_R &= ~0x11; // Alle echten LEDs D3/D4 aus
}
// //############################################################################
// //###                                                                      ###
// //###      AUFGABE 4: LED Steuerung über UART                              ###
// //###                                                                      ###
// //############################################################################
// // #include "inc/tm4c1294ncpdt.h" 			// Header of the controller type
// #include "tm4c1294ncpdt.h"
// #include <stdio.h>
// #include <stdint.h>

// #define IDLETIME 1000
// #define MAXSIZE 50
// #define PORTM 0x00000800

// #define ERROR_NONE 0
// #define ERROR_TOO_SHORT 1
// #define ERROR_WRONG_PREFIX 2
// #define ERROR_INVALID_OPERATOR 3
// #define ERROR_INVALID_LED_NUMBER 4
// #define ERROR_TOO_LONG 5

// volatile int wait_counter_4 = 0; // 'volatile' verhindert, dass der Compiler die Warteschleife optimiert.

// void config_port_aufgabe_4_optional(void) {
//     // Clock für Port P (UART), Port N (LEDs vom Tiva), Port F (echte LEDs) und Port M (Simulation)
//     SYSCTL_RCGCGPIO_R |= 0x02000; // switch on clock for Port P
//     SYSCTL_RCGCGPIO_R |= 0x00000800;   // switch on clock for Port M
    
//     // wait_counter_4++;
//     while ((SYSCTL_PRGPIO_R & 0x02000) == 0) {
//     }; // Wait until bit 13 of Port P is set. Bit-shift //  while((SYSCTL_PRGPIO_R
//      // & (1 << 13)) == 0) {};
    
//     SYSCTL_RCGCGPIO_R |= 0x1000; // |= (1 << 12); // Takt für Port N (LEDs D1, D2) (entspricht 1 << 12)
//     SYSCTL_RCGCGPIO_R |= 0x0020; // |= (1 << 5);  // Takt für Port F (LEDs D3, D4) (entspricht 1 << 5)
//     SYSCTL_RCGCGPIO_R |= 0x0800; // |= (1 << 11); // TODO: Takt für Port M ANFxx (entspricht 1 << 11)

//     // Warte auf Taktversorgung für die GPIO-Ports stabil ist,
//     // bevor auf deren Register zugegriffen wird.
//     while((SYSCTL_PRGPIO_R & 0x1000) == 0) {}; // Warte, bis Bit 12 von Port N gesetzt ist. Bit-shift //  while((SYSCTL_PRGPIO_R & (1 << 12)) == 0) {};
//     while((SYSCTL_PRGPIO_R & 0x0020) == 0) {};  // Warte auf Port F (LEDs D3, D4) (entspricht 1 << 5)


//     // Port P für UART6 (Rx/Tx) konfigurieren
//     GPIO_PORTP_DEN_R |= 0x03;       // PP1 & PP0 digital aktivieren
//     GPIO_PORTP_DIR_R |= 0x02;       // PP1 (Tx) als Ausgang
//     GPIO_PORTP_DIR_R &= ~0x01;      // PP0 (Rx) als Eingang
//     GPIO_PORTP_AFSEL_R |= 0x03;     // Alternate Function für PP1 & PP0
//     GPIO_PORTP_PCTL_R = (GPIO_PORTP_PCTL_R & 0xFFFFFF00) | 0x00000011;

//     // Port N für ECHTE LEDs D1 (PN1) und D2 (PN0) konfigurieren
//     GPIO_PORTN_DEN_R |= 0x03;       // PN1 & PN0 digital aktivieren
//     GPIO_PORTN_DIR_R |= 0x03;       // PN1 & PN0 als Ausgänge

//     // Port F für ECHTE LEDs D3 (PF4) und D4 (PF0) konfigurieren
//     GPIO_PORTF_AHB_DEN_R |= 0x11;   // PF4 & PF0 digital aktivieren
//     GPIO_PORTF_AHB_DIR_R |= 0x11;   // PF4 & PF0 als Ausgänge
    
//     // Port M für die "SIMULATION" laut Anleitung konfigurieren
//     GPIO_PORTM_DEN_R |= 0x0F;       // PM0-PM3 digital aktivieren
//     GPIO_PORTM_DIR_R |= 0x0F;       // PM0-PM3 als Ausgänge
// }

// void config_uart_aufgabe_4_optional(uint32_t baudrate, uint32_t lcrh_setting) {
//     // #define BPS 115200 // Aufgabe 4 nutzt oft eine höhere Baudrate
//     SYSCTL_RCGCUART_R |= 0x40;      // switch on clock for UART6
//     wait_counter_4++;                           // delay for stable clock
//     while((SYSCTL_PRUART_R & 0x40) == 0) {};
//     UART6_CTL_R &= ~0x01;           // disable UART6 for config
//     UART6_IBRD_R = 8;               // set DIVINT of BRD floor(16 MHz / (16 * 115200 bps)) = 8
//     UART6_FBRD_R = 44;              // set DIVFRAC of BRD = round(0.6805 * 64) = 44
//     // UART6_LCRH_R = 0x00000060;      // serial format 8N1
//     UART6_LCRH_R = lcrh_setting; // Parameter verwenden
//     UART6_CTL_R |= 0x0301;          // UART transmit/receive on and UART enable
// }

// void send_char4_optional(char zeichen_zum_senden) {
//     // Aktives Warten (Polling), solange der Sende-Puffer voll ist.
//     // Das TXFF-Flag (Bit 5) im Flag-Register ist 1, solange der Puffer voll ist.
//     while ((UART6_FR_R & 0x20) != 0);
//     // Wenn die Schleife beendet wurde, ist Platz im Puffer.
//     // Schreibe das zu sendende Zeichen in das Datenregister.
//     UART6_DR_R = zeichen_zum_senden;
// }

// // --- Main-Funktion für Aufgabe 4
// void aufgabe_4_optional(uint32_t baudrate) {
//     char empfange_zeichen;
//     char buffer[MAXSIZE];
//     volatile int i;
//     config_port_aufgabe_4();
//     config_uart_aufgabe_4(baudrate, 0x00000060); // Provide baudrate and LCRH setting
//     // Initialzustand: Alle LEDs einschalten
//     GPIO_PORTM_DATA_R |= 0x0F;      // Schaltet simulierte LEDs an Port M an
//     GPIO_PORTN_DATA_R |= 0x03;      // Schaltet echte LEDs D1 (PN1) und D2 (PN0) an
//     GPIO_PORTF_AHB_DATA_R |= 0x11;  // Schaltet echte LEDs D3 (PF4) und D4 (PF0) an

//     while (1) {                     // Hauptschleife
//         // Sendet den Prompt ">" in einer neuen Zeile an das Terminal
//         send_char4_optional('\r'); // Zeilenumbruch
//         send_char4_optional('\n'); // Neue Zeile
//         send_char4_optional('>');  // Sendet das Zeichen '>' 

//         i = 0;
//         while (i < MAXSIZE - 1) {
//             // Wartet solange der Empfangs-Puffer (FIFO) leer ist.
//             // Die Maske 0x10 prüft Bit 4. Solange dieses Bit 1 ist (Puffer leer),
//             while ((UART6_FR_R & 0x10) != 0); // Warte Loop
//             empfange_zeichen = UART6_DR_R; // Das empfangene Zeichen aus dem Datenregister holen
//             if (empfange_zeichen == '\r' // Abbruch bei Carriage Return oder End-Of-Transmission (EOT)
//                 || 
//                 empfange_zeichen == 0x04) 
//                 break; 
//             buffer[i++] = empfange_zeichen; // Das gültige Zeichen in den Puffer schreiben und den Zähler für die nächste Position erhöhen.
//         }
//         buffer[i] = '\0'; // Den String mit einem Null-Terminator abschließen. Das ist wichtig.
//         printf("Befehl empfangen: %s\n", buffer);
//         // Befehl parsen und auf alle relevanten Ports anwenden
//         if (buffer[0] == 'l' && buffer[1] == 'e' && buffer[2] == 'd') {
//             char operation = buffer[3]; // '+' oder '-'
//             char led_num = buffer[4];   // '0' bis '3'

//             if (operation == '+') {
//                 switch (led_num) {
//                     case '0':
//                         GPIO_PORTF_AHB_DATA_R |= 0x01; // Echte LED D4 (PF0) an
//                         GPIO_PORTM_DATA_R     |= 0x01; // Simulation für PM0 an
//                         break;
//                     case '1':
//                         GPIO_PORTF_AHB_DATA_R |= 0x10; // Echte LED D3 (PF4) an
//                         GPIO_PORTM_DATA_R     |= 0x02; // Simulation für PM1 an
//                         break;
//                     case '2':
//                         GPIO_PORTN_DATA_R     |= 0x01; // Echte LED D2 (PN0) an
//                         GPIO_PORTM_DATA_R     |= 0x04; // Simulation für PM2 an
//                         break;
//                     case '3':
//                         GPIO_PORTN_DATA_R     |= 0x02; // Echte LED D1 (PN1) an
//                         GPIO_PORTM_DATA_R     |= 0x08; // Simulation für PM3 an
//                         break;
//                 }
//             } else if (operation == '-') {
//                 switch (led_num) {
//                     case '0':
//                         GPIO_PORTF_AHB_DATA_R &= ~0x01; // Echte LED D4 (PF0) aus
//                         GPIO_PORTM_DATA_R     &= ~0x01; // Simulation für PM0 aus
//                         break;
//                     case '1':
//                         GPIO_PORTF_AHB_DATA_R &= ~0x10; // Echte LED D3 (PF4) aus
//                         GPIO_PORTM_DATA_R     &= ~0x02; // Simulation für PM1 aus
//                         break;
//                     case '2':
//                         GPIO_PORTN_DATA_R     &= ~0x01; // Echte LED D2 (PN0) aus
//                         GPIO_PORTM_DATA_R     &= ~0x04; // Simulation für PM2 aus
//                         break;
//                     case '3':
//                         GPIO_PORTN_DATA_R     &= ~0x02; // Echte LED D1 (PN1) aus
//                         GPIO_PORTM_DATA_R     &= ~0x08; // Simulation für PM3 aus
//                         break;
//                 }
//             }
//         }
//     }
// }


// void remove_spaces(char* str) {
//     if (str == NULL) return;
//     char* write_ptr = str; // Zeiger, wohin das nächste Nicht-Leerzeichen geschrieben wird
//     char* read_ptr = str;  // Zeiger, der den String liest

//     while (*read_ptr) {
//         if (*read_ptr != ' ') {
//             *write_ptr = *read_ptr;
//             write_ptr++;
//         }
//         read_ptr++;
//     }
//     *write_ptr = '\0'; // Null-Terminator am neuen Ende des Strings setzen
// }

// int parse_and_execute_led_command_optional(char* buffer) { // buffer ist jetzt nicht mehr const, da remove_spaces ihn modifiziert
//     int error_code = ERROR_NONE;
    
//     // Optionale Erweiterung: Beliebig viele Leerzeichen erlauben
//     // Schritt 1: Leerzeichen aus dem Puffer entfernen
//     remove_spaces(buffer);

//     // Jetzt die Länge des modifizierten Puffers ohne Leerzeichen bestimmen
//     int len = 0;
//     while(buffer[len] != '\0') {
//         len++;
//     }

//     // Syntaxprüfung: "led<+|-><0|1|2|3>" (jetzt ohne Leerzeichen)
//     // Mindestlänge für "led+0" ist 5 Zeichen
//     if (len < 5) {
//         return ERROR_TOO_SHORT;
//     }
//     // Maximale Länge für einen einzelnen Befehl ohne Leerzeichen ist 5 ("led+0")
//     // Wenn wir später kombinierte Befehle erlauben, muss diese Logik angepasst werden.
//     // Für jetzt: Ein einzelner Befehl wird erwartet.
//     if (len > 5 && len != 0) { // len != 0, falls buffer nur aus Leerzeichen bestand
//         // Diese Prüfung ist für die aktuelle Anforderung (einzelne LED)
//         // Wenn "led+1+2-3" erlaubt werden soll, wird das hier komplexer.
//         return ERROR_TOO_LONG; // Oder einen spezifischeren Fehlercode für ungültiges Format
//     }
//     if (len == 0) { // Falls der Buffer nur aus Leerzeichen bestand
//         return ERROR_TOO_SHORT; // Oder einen anderen passenden Fehlercode
//     }


//     if (buffer[0] != 'l' || buffer[1] != 'e' || buffer[2] != 'd') {
//         return ERROR_WRONG_PREFIX;
//     }

//     char operation = buffer[3];
//     char led_num_char = buffer[4];

//     if (operation != '+' && operation != '-') {
//         return ERROR_INVALID_OPERATOR;
//     }

//     if (led_num_char < '0' || led_num_char > '3') {
//         return ERROR_INVALID_LED_NUMBER;
//     }

//     // Wenn alle Prüfungen bestanden, führe die Aktion aus
//     if (operation == '+') {
//         switch (led_num_char) {
//             case '0':
//                 GPIO_PORTF_AHB_DATA_R |= 0x01; // Echte LED D4 (PF0) an
//                 GPIO_PORTM_DATA_R     |= 0x01; // Simulation für PM0 an
//                 break;
//             case '1':
//                 GPIO_PORTF_AHB_DATA_R |= 0x10; // Echte LED D3 (PF4) an
//                 GPIO_PORTM_DATA_R     |= 0x02; // Simulation für PM1 an
//                 break;
//             case '2':
//                 GPIO_PORTN_DATA_R     |= 0x01; // Echte LED D2 (PN0) an
//                 GPIO_PORTM_DATA_R     |= 0x04; // Simulation für PM2 an
//                 break;
//             case '3':
//                 GPIO_PORTN_DATA_R     |= 0x02; // Echte LED D1 (PN1) an
//                 GPIO_PORTM_DATA_R     |= 0x08; // Simulation für PM3 an
//                 break;
//         }
//     } else if (operation == '-') {
//         // ... (Ausschaltlogik wie gehabt) ...
//         switch (led_num_char) {
//             case '0':
//                 GPIO_PORTF_AHB_DATA_R &= ~0x01; // Echte LED D4 (PF0) aus
//                 GPIO_PORTM_DATA_R     &= ~0x01; // Simulation für PM0 aus
//                 break;
//             case '1':
//                 GPIO_PORTF_AHB_DATA_R &= ~0x10; // Echte LED D3 (PF4) aus
//                 GPIO_PORTM_DATA_R     &= ~0x02; // Simulation für PM1 aus
//                 break;
//             case '2':
//                 GPIO_PORTN_DATA_R     &= ~0x01; // Echte LED D2 (PN0) aus
//                 GPIO_PORTM_DATA_R     &= ~0x04; // Simulation für PM2 aus
//                 break;
//             case '3':
//                 GPIO_PORTN_DATA_R     &= ~0x02; // Echte LED D1 (PN1) aus
//                 GPIO_PORTM_DATA_R     &= ~0x08; // Simulation für PM3 aus
//                 break;
//         }
//     }
//     return error_code; // ERROR_NONE wenn alles gut ging
// }

// // execute-Funktion: Passt jetzt zur Signatur in aufgaben.h
// void execute_aufgabe_4_optional(char zeichen, uint32_t baudrate, uint32_t lcrh_setting) {
//     static char buffer[MAXSIZE]; // MAXSIZE ist in aufgabe_4.c auf 10 gesetzt, das ist knapp für "led+0" + Null + evtl. Leerzeichen
//     static int i = 0;
//     char c;
//     static uint32_t last_baudrate = 0;
//     static uint32_t last_lcrh = 0;
//     static uint8_t is_initialized = 0;
//     int parse_error_code = ERROR_NONE;

//     // UART konfigurieren, falls sich Parameter geändert haben oder beim allerersten Aufruf
//     if (baudrate != last_baudrate || lcrh_setting != last_lcrh) {
//         config_uart_aufgabe_4_optional(baudrate, lcrh_setting); // Ihre bestehende UART-Konfig
//         last_baudrate = baudrate;
//         last_lcrh = lcrh_setting;
//         is_initialized = 0; // Erzwingt neuen Prompt nach Neukonfiguration
//     }

//     if (!is_initialized) {
//         send_char4_optional('\r'); // Ihre bestehende send_char Funktion
//         send_char4_optional('\n');
//         send_char4_optional('>');
//         is_initialized = 1;
//     }

//     // Prüfen, ob ein Zeichen im Empfangspuffer ist
//     if ((UART6_FR_R & UART_FR_RXFE) == 0) { // UART_FR_RXFE ist 0x10
//         c = UART6_DR_R;

//         if (c == '\r' || c == 0x04) { // Enter oder EOT
//             buffer[i] = '\0';
//             send_char4_optional('\r'); // Echo der Eingabezeile beenden
//             send_char4_optional('\n');

//             parse_error_code = parse_and_execute_led_command_optional(buffer);

//             // Anforderung 5 & Optionale Erweiterung: Ausgabe mit printf() und Errorcode
//             printf("Empfangener Befehl: \"%s\", Errorcode: %d\n", buffer, parse_error_code);

//             // Anforderung 3: Bei Syntaxfehler wird Eingabe ignoriert (passiert durch Fehlerprüfung oben)
//             // Anforderung 4: Neuer Prompt
//             i = 0; // Buffer für nächste Eingabe zurücksetzen
//             send_char4_optional('>');
//         } else if (c == 0x08 || c == 0x7F) { // Backspace / DEL
//             if (i > 0) {
//                 i--;
//                 send_char4_optional(0x08); // Backspace
//                 send_char4_optional(' ');  // Überschreibe Zeichen
//                 send_char4_optional(0x08); // Cursor zurück
//             }
//         } else if (i < MAXSIZE - 1) {
//             send_char4_optional(c); // Echo des Zeichens
//             buffer[i++] = c;
//         } else {
//             // Puffer voll, aber noch kein Enter. Optional: Fehler signalisieren oder Zeichen ignorieren
//             // Für jetzt: Ignorieren, um Überlauf zu verhindern.
//             // Man könnte hier auch einen Warnton senden o.ä.
//         }
//     }
// }
