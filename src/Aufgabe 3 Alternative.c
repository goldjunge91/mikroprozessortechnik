//===========================================================
// Aufgabe 3
// Aufgabe 1a: 7 Datenbits, Even Parity, 1 Stopbit (7E1) @ konfigurierbarer Baudrate über Port PP1
//==========================================================

// #include "inc/tm4c1294ncpdt.h"          // Header of the controller type
#include "tm4c1294ncpdt.h"
#include <stdint.h>                     // Header w. types for the register ..
#include <stdio.h>

#define IDLETIME 1000                   // waiting time between transmissions
#define MAXSIZE 10                      // ANF4 Max size of Array
#define SYSTEM_CLOCK 16000000U          // Systemtakt in Hz (Annahme: 16 MHz ohne PLL)

static volatile uint32_t wait_counter_3_alt = 0; // volatile, um Compiler-Optimierungen zu verhindern, die eine leere Schleife entfernen würden.

/**
 * @brief Konfiguriert Port P für die UART6 TX Funktionalität.
 *
 * Aktiviert den Takt für Port P und konfiguriert Pin PP1 als alternierende Funktion U6TX.
 * Pin PP1 wird als digitaler Ausgang mit aktivierter Alternativfunktion für UART6 Transmit konfiguriert.
 */
void config_port_3_alt(){
    // initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000;       // switch on clock for Port P
    wait_counter_3_alt++;                               // delay for stable clock
    // initialize Port P
    GPIO_PORTP_DEN_R |= 0x03;           // enable digital pin function for PP1 & PP0
    GPIO_PORTP_DIR_R |= 0x02;           // set PP1 to output
    GPIO_PORTP_DIR_R &= ~0x01;          // set PP0 to input
    GPIO_PORTP_AFSEL_R |= 0x03;         // switch to alternate pin function PP1 & PP0
    GPIO_PORTP_PCTL_R |= 0x10;          // select alternate pin function PP1->U6Tx
    GPIO_PORTP_PCTL_R |= 0x01;          // select alternate pin function PP0->U6Rx
}

void config_uart_3_alt(uint32_t baudrate, uint32_t lcrh_setting){
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40;          // switch on clock for UART6
    wait_counter_3_alt++;                               // delay for stable clock
    UART6_CTL_R &= ~0x01;               // disable UART6 for config
    // initialize bitrate of 115200 bit per second
    UART6_IBRD_R = 16000000/(16*baudrate);   // set DIVINT of BRD floor(16 MHz/16*9600 bps)
    UART6_FBRD_R = 44;                  // set DIVFRAC of BRD remaining fraction divider
    UART6_LCRH_R = lcrh_setting; // Parameter wird verwendet 
    UART6_LCRH_R = 0x00000060;          // serial format 8N1
    UART6_CTL_R |= 0x0301;              // UART transmit and receive on and UART enable
}

void idle_3_alt() {                           // simple wait for idle_3_alt state
    int i;
    for (i=IDLETIME;i>0;i--);           // count down loop for waiting
}

void send_3_alt(char c){
    while((UART6_FR_R & 0x20) !=0);     // till transmit FIFO not full
    UART6_DR_R = c;                     // sendet Zeichen
    idle_3_alt();                             // idle_3_alt time
}

void run_aufgabe_3_alt(char zeichen, uint32_t baudrate){
    char buffer[MAXSIZE];               // ANF3 Um aus den Daten einen Array zuerstellen
    char c;                             // Um Daten zu empfangen
    int i;
    config_port_3_alt();                      // configuration of Port P
    config_uart_3_alt(baudrate, 0x00000060);
    while(1){
        // ANF1 und ANF7 neuer Prompt
        send_3_alt(0x0D);                     // send_3_alt the character '\r'
        send_3_alt(0x0A);                     // send_3_alt the character '\n'
        send_3_alt('>');                      // send_3_alt the character '>'
        i = 0;                          // Buffer-Index zuruecksetzen
        while(i < MAXSIZE-1){           // ANF5 minus 1 fuer Platz fuer 0x00 
            while(UART6_FR_R & 0x10);   // ANF2 es wird auf Daten gewartet
            c = UART6_DR_R;             // Daten werden Abgelesen um auf EOT zu prüfen
            if (c == 0x04) break;       // EOT im ASCII Code 0x04 wird erkannt und die Schleife beendet if (i == MAXSIZE-1) break;
            if (c == 0x0D) break;       // ANF5 0x0D       
            buffer[i] = c;              // Zeichen speichern
            i++;
        }
        buffer[i] = 0x00;               // String beenden
        printf("Empfangene Nachricht: %s\n", buffer);          // ANF6
    }
}

// init-Funktion: Wird einmalig beim Umschalten auf diese Aufgabe aufgerufen
void init_aufgabe_3_alt(void) {
    config_port_3_alt();
    // config_uart_3_alt();
    // // Prompt beim Start der Aufgabe senden
    // send_3_alt(0x0D);                     // send_3_alt the character ('\r');
    // send_3_alt(0x0A);                     // send_3_alt the character ('\n');
    // send_3_alt('>');                      // send_3_alt the character ('>');
    // send_3_alt('\r');
    // send_3_alt('\n');
    // send_3_alt('>');
}

// execute-Funktion: Wird in der Hauptschleife kontinuierlich aufgerufen
void execute_aufgabe_3_alt(char zeichen, uint32_t baudrate, uint32_t lcrh_setting) {
    // Statische Variablen, um den Zustand des Buffers zwischen den Aufrufen zu speichern
    static char buffer[MAXSIZE];
    static int i = 0;
    static uint32_t last_baudrate = 0;
    static uint32_t last_lcrh = 0;
    static uint8_t is_initialized = 0;
    char c;

        // UART konfigurieren, falls sich Parameter geändert haben oder beim allerersten Aufruf
    if (baudrate != last_baudrate || lcrh_setting != last_lcrh) {
        config_uart_3_alt(baudrate, lcrh_setting);
        last_baudrate = baudrate;
        last_lcrh = lcrh_setting;
        is_initialized = 0; // Erzwingt neuen Prompt nach Neukonfiguration
    }
    // Beim ersten Durchlauf nach der Initialisierung einen Prompt senden
    if (!is_initialized) {
            // send_3_alt('\r');
            // send_3_alt('\n');
            // send_3_alt('>');
            send_3_alt(0x0D);                     // send_3_alt the character ('\r');
            send_3_alt(0x0A);                     // send_3_alt the character ('\n');
            send_3_alt('>');                      // send_3_alt the character ('>');
        is_initialized = 1;
    }

    // Prüfen, ob ein Zeichen empfangen wurde (nicht-blockierend)
    if ((UART6_FR_R & 0x10) == 0) {
        c = UART6_DR_R;

        // Echo an den Benutzer
        send_3_alt(c);

        // if (c == '\r' || c == 0x04) { // Eingabe beendet mit Enter oder EOT
    if (c == 0x04 || c == 0x0D) {
            buffer[i] = '\0';
            printf("Aufgabe 3 empfing: %s\n", buffer); // Ausgabe auf Debug-Konsole
            i = 0; // Buffer zurücksetzen

            // Neuen Prompt senden
            // send_3_alt('\r');
            // send_3_alt('\n');
            // send_3_alt('>');
            send_3_alt(0x0D);                     // send_3_alt the character ('\r');
            send_3_alt(0x0A);                     // send_3_alt the character ('\n');
            send_3_alt('>');                      // send_3_alt the character ('>');
        } else if (i < MAXSIZE - 1) {
            buffer[i++] = c; // Zeichen im Puffer speichern
        }
    }
}
    // Prüfen, ob ein Zeichen im Empfangspuffer ist (nicht-blockierend)
    // if ((UART6_FR_R & 0x10) == 0) {
    //     c = UART6_DR_R; // Zeichen lesen

    //     // Prüfen, ob die Eingabe beendet ist (Enter oder EOT)
    //     //   if (c == 0x04) break;       // EOT im ASCII Code 0x04 wird erkannt und die Schleife beendet if (i == MAXSIZE-1) break;
    //     //     if (c == 0x0D) break;       // ANF5 0x0D      
    //     // if (c == '\r' || c == 0x04) {
    //     if (c == 0x04 || c == 0x0D) {
    //         buffer[i] = '\0'; // String abschließen
            
    //         // Hier die "verarbeitete" Nachricht ausgeben.
    //         // In einem echten System würde man sie vielleicht zurücksenden.
    //         // Das printf geht an die Debug-Konsole, nicht an die UART.
    //         printf("Aufgabe 3 empfing: %s\n", buffer);

    //         // Buffer-Index zurücksetzen für die nächste Eingabe
    //         i = 0;

    //         // Neuen Prompt senden
    //         send_3_alt('\r');
    //         send_3_alt('\n');
    //         send_3_alt('>');

    //     } else if (i < MAXSIZE - 1) {
    //         // Echo: das empfangene Zeichen direkt zurücksenden, damit der Benutzer sieht, was er tippt
    //         send_3_alt(c);
    //         // Zeichen im Puffer speichern
    //         buffer[i] = c;
    //         i++;
    //     }
    // }
    // Wenn kein Zeichen da ist, tut die Funktion nichts und kehrt sofort zurück.
// }
