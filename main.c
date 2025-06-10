#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>
#include "src/aufgaben.h" // Stellt sicher, dass die Konfig-Funktionen bekannt sind

// Diese Funktion ist nur für den Test. Sie müssen die Definition aus
// run_aufgabe_4.c nicht kopieren, da der Linker sie findet.
void config_port_aufgabe4(void);
void config_uart_aufgabe4(void);


void main(void)

{
    run_aufgabe_4();

    // =======================================================================
    // ===                 TEMPORÄRER CODE ZUM TESTEN                      ===
    // =======================================================================

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
    config_port_aufgabe4(); // Konfiguriert Port P für UART
    config_uart_aufgabe4(); // Konfiguriert UART6 selbst


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

        // Schalte die LED D1 um (an/aus). Dies ist Ihr sichtbares Zeichen,
        // dass der Test fehlgeschlagen ist und das Board ein Hardware-Problem hat.
        GPIO_PORTN_DATA_R ^= 0x02;
    }
}
// //===========================================================
// // Example Program for the TIVA TM4C1294XL Evaluation Board
// // With this file the LEDs D0 to D3 on the eval board will
// // blink each one after another. To control the LEDs the
// // Ports N and F with Pins PN0, PN1, PF0 and PF4 have to be
// // enabled and set as digital output.
// //===========================================================
// // Include the Header File for controller tm4c1294ncpdt
// #include "inc/tm4c1294ncpdt.h"
// #include <stdint.h>
// #include <stdio.h>
// #include "src/aufgaben.h"

// #define AUFGABE_NUMMER 4  // choose task

// void main(void) {
//     switch (AUFGABE_NUMMER) {
//         case 1:
//             printf("Aufgabe 1a\n");
//             run_aufgabe_1a();
//             break;
//             case 2:
//             printf("Aufgabe 1b\n");
//             run_aufgabe_1b();
//             break;
//             case 3:
//             printf("Aufgabe 1c\n");
//             run_aufgabe_1c();
//             break;
//         case 31:
//             // run_aufgabe_3();
//             run_aufgabe_3_alt();
//             break;
//         case 32:
//             aufgabe_3_Final_v2();
//             break;
//         case 4:
//             run_aufgabe_4();
//             break;
//         case 41:
//             labor_aufgabe_4();
//             break;

//         default:
//             // eine LED blinken lassen, um einen Fehler zu signalisieren.
//             while(1);
//             break;
//     }
// }

// // void main(void)
// // {
// //     int i=0;
// //     SYSCTL_RCGCGPIO_R = 0x00001020 ;
// //     i++;
// //     GPIO_PORTN_DEN_R        = 0x03;
// //     GPIO_PORTN_DIR_R        = 0x03;
// //     GPIO_PORTF_AHB_DEN_R    = 0x11;
// //     GPIO_PORTF_AHB_DIR_R    = 0x11;
// //     while(1)
// //     {
// //         GPIO_PORTN_DATA_R = 0x02;
// //         for(i=0;i<500000;i++);

// //         GPIO_PORTN_DATA_R = 0x01;
// //         for(i=0;i<500000;i++);
// //         GPIO_PORTN_DATA_R = 0x00;

// //         GPIO_PORTF_AHB_DATA_R = 0x10;
// //         for(i=0;i<500000;i++);

// //         GPIO_PORTF_AHB_DATA_R = 0x01;
// //         for(i=0;i<500000;i++);

// //         GPIO_PORTF_AHB_DATA_R = 0x00;
// //     }
// // }



// //void main(void)
// //{
// //int i=0;
// //SYSCTL_RCGCGPIO_R = 0x00001020 ;
// //i++;
// //GPIO_PORTN_DEN_R = 0x03;
// //GPIO_PORTN_DIR_R = 0x03;
// //GPIO_PORTF_AHB_DEN_R = 0x11;
// //GPIO_PORTF_AHB_DIR_R = 0x11;
// //while(1)
// //{
// //GPIO_PORTN_DATA_R = 0x02;
// //for(i=0;i<500000;i++);
// //GPIO_PORTN_DATA_R = 0x01;
// //for(i=0;i<500000;i++);
// //GPIO_PORTN_DATA_R = 0x00;
// //GPIO_PORTF_AHB_DATA_R = 0x10;
// //for(i=0;i<500000;i++);
// //GPIO_PORTF_AHB_DATA_R = 0x01;
// //for(i=0;i<500000;i++);
// //GPIO_PORTF_AHB_DATA_R = 0x00;
// //}
// //}
