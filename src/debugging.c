// =======================================================================
// ===                 TEMPORÄRER CODE ZUM TESTEN                      ===
// =======================================================================
#include "aufgaben.h"
#include "inc/tm4c1294ncpdt.h" 			// Header of the controller type
#include <stdint.h> 				// Header w. types for the register ..

#define BPS 115200
static int wait_counter_debug = 0;

void config_port_debugging(void) {
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

void config_uart_debugging(void) {
    SYSCTL_RCGCUART_R |= 0x40;      // switch on clock for UART6
    wait_counter_debug++; 
    UART6_CTL_R &= ~0x01;           // disable UART6 for config
    UART6_IBRD_R = 8;               // set DIVINT of BRD floor(16 MHz / (16 * 115200 bps)) = 8
    UART6_FBRD_R = 44;              // set DIVFRAC of BRD = round(0.6805 * 64) = 44
    UART6_LCRH_R = 0x00000060;      // serial format 8N1
    UART6_CTL_R |= 0x0301;          // UART transmit/receive on and UART enable
}

void debugging(){
    // Schritt 1: On-Board LED D1 (Pin PN1) als visuellen Indikator konfigurieren
    // ----------------------------------------------------------------------------
    SYSCTL_RCGCGPIO_R |= (1 << 12); // Takt für Port N aktivieren
    // Warten, bis der Takt für den Port stabil ist
    while((SYSCTL_PRGPIO_R & (1 << 12)) == 0) {};
    GPIO_PORTN_DIR_R |= 0x02;    // Pin PN1 (verbunden mit LED D1) als Ausgang setzen
    GPIO_PORTN_DEN_R |= 0x02;    // Digitale Funktion für PN1 aktivieren
    GPIO_PORTN_DATA_R &= ~0x02;  // LED D1 zu Beginn explizit ausschalten


    // Schritt 2: UART6 wie in Aufgabe 4 konfigurieren
    // ----------------------------------------------------------------------------
    config_port_debugging(); // Konfiguriert Port P für UART
    config_uart_debugging(); // Konfiguriert UART6 selbst


    // Schritt 3: Endlosschleife, die auf "Geister-Daten" wartet
    // ----------------------------------------------------------------------------
    while(1)
    {
        // Diese innere Schleife wartet, solange der Empfangspuffer (FIFO) leer ist.
        // Das Flag 0x10 (RXFE) im Flag-Register (FR) ist 1, wenn der Puffer leer ist.
        // Das Programm sollte hier "hängen" bleiben, solange nichts empfangen wird.
        while((UART6_FR_R & 0x10) != 0)
        {
            // Bleibe hier, solange kein Zeichen ankommt.
        }

        // ---- Wenn der Code diesen Punkt erreicht, ist etwas schiefgelaufen! ----
        // Es wurde ein Zeichen "empfangen", obwohl nichts angeschlossen ist.
        // Lese das "Geister-Zeichen" aus dem Datenregister, um den Fehler-Zustand
        // für den nächsten Schleifendurchlauf zurückzusetzen.
        volatile char temp = UART6_DR_R;

        // Schalte die LED D1 um (an/aus). Led indikator,
        // dass der Test fehlgeschlagen ist.
        GPIO_PORTN_DATA_R ^= 0x02;
    }
}
