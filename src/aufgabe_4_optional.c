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


void config_port_aufgabe_4_optional(void) {
    // Clock für Port P (UART), Port N (LEDs D1, D2), Port F (LEDs D3, D4) und Port M (Simulation)
    // Port P (UART6) ist Bit 13 -> 0x2000
    // Port N (LEDs D1, D2) ist Bit 12 -> 0x1000
    // Port F (LEDs D3, D4) ist Bit 5 -> 0x0020
    // Port M (Simulation PM0-PM3) ist Bit 11 -> 0x0800
    SYSCTL_RCGCGPIO_R |= 0x2000; // Takt für Port P (UART6)
    SYSCTL_RCGCGPIO_R |= 0x1000; // Takt für Port N (LEDs D1, D2)
    SYSCTL_RCGCGPIO_R |= 0x0020; // Takt für Port F (LEDs D3, D4)
    SYSCTL_RCGCGPIO_R |= 0x0800; // Takt für Port M (Simulation PM0-PM3)

    // Warte, bis die Taktversorgung für die GPIO-Ports stabil ist
    while ((SYSCTL_PRGPIO_R & 0x2000) == 0) {}; // Warten auf Port P
    while ((SYSCTL_PRGPIO_R & 0x1000) == 0) {}; // Warten auf Port N
    while ((SYSCTL_PRGPIO_R & 0x0020) == 0) {}; // Warten auf Port F
    while ((SYSCTL_PRGPIO_R & 0x0800) == 0) {}; // Warten auf Port M

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
    // UART6 ist Bit 6 -> 0x0040
    SYSCTL_RCGCUART_R |= 0x0040;      // Takt für UART6 (Bit 6) einschalten
    while((SYSCTL_PRUART_R & 0x0040) == 0) {}; // Warten, bis UART6-Peripherie bereit ist
    
    UART6_CTL_R &= ~UART_CTL_UARTEN; // UART6 für Konfiguration deaktivieren (UART_CTL_UARTEN ist 0x1)

    // Baudrate bewegt in Task-Switcher
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
    if (len < 3) { // Mindestlänge für "led"
        return ERROR_TOO_SHORT;
    }

    if (buffer[0] != 'l' || buffer[1] != 'e' || buffer[2] != 'd') {
        return ERROR_WRONG_PREFIX;
    }

    if (len == 3) { // Nur "led" ohne Operationen
        return ERROR_TOO_SHORT; // Keine Operationen angegeben
    }

    // Der Teil des Befehls nach "led" muss eine gerade Länge haben (Paare von Op+Num)
    int operations_len = len - 3;
    if (operations_len % 2 != 0) {
        return ERROR_INVALID_OPERATOR; // Oder ein neuer Fehlercode ERROR_INVALID_COMMAND_FORMAT
    }

    char* cmd_ptr = buffer + 3; // Zeiger auf den Beginn der Operationen

    while (*cmd_ptr != '\0') {
        char operation = *cmd_ptr;
        cmd_ptr++; // Zum nächsten Zeichen (LED-Nummer)
        // Es wurde bereits geprüft, dass operations_len gerade ist, 
        // daher sollte *cmd_ptr hier nicht '\0' sein, wenn die Logik korrekt ist.
        // Eine zusätzliche Sicherheitsprüfung schadet aber nicht:
        if (*cmd_ptr == '\0') { 
            // Sollte durch die Längenprüfung oben abgedeckt sein, aber als Fallback
            return ERROR_INVALID_OPERATOR; // Operator ohne Nummer am Ende
        }
        char led_num_char = *cmd_ptr;
        cmd_ptr++; // Zum nächsten Operator oder String-Ende

        if (operation != '+' && operation != '-') {
            return ERROR_INVALID_OPERATOR;
        }

        if (led_num_char < '0' || led_num_char > '3') {
            return ERROR_INVALID_LED_NUMBER;
        }

        // Erfolgreich geparst für dieses Segment, führe die Aktion aus
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
    }
    return ERROR_NONE; // Kein Fehler, wenn alle Segmente erfolgreich waren
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
    // Initialzustand der LEDs setzen, falls gewünscht:
    // GPIO_PORTM_DATA_R = 0x00; // Alle Simulations-LEDs aus
    // GPIO_PORTN_DATA_R = 0x00; // Alle echten LEDs D1/D2 aus
    // GPIO_PORTF_AHB_DATA_R &= ~0x11; // Alle echten LEDs D3/D4 aus
}
