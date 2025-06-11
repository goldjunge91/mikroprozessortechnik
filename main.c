//===========================================================
// Example Program for the TIVA TM4C1294XL Evaluation Board
// With this file the LEDs D0 to D3 on the eval board will
// blink each one after another. To control the LEDs the
// Ports N and F with Pins PN0, PN1, PF0 and PF4 have to be
// enabled and set as digital output.
//===========================================================
// Include the Header File for controller tm4c1294ncpdt
#include "inc/tm4c1294ncpdt.h"
#include "src/aufgaben.h"
#include <stdint.h>
#include <stdio.h>

//===========================================================
// HIER DIE GEWÜNSCHTE AUFGABE AUSWÄHLEN
//
// 11 = Aufgabe 1a
// 12 = Aufgabe 1b
// 13 = Aufgabe 1c
// 30 = Aufgabe 3
// 40 = Aufgabe 4
//===========================================================
#define AUFGABE_NUMMER 30

void main(void) {
    switch (AUFGABE_NUMMER) {
        case 11:
            printf("Führe Aufgabe 1a aus...\n");
            run_aufgabe_1a();
            break;

        case 12:
            printf("Führe Aufgabe 1b aus...\n");
            run_aufgabe_1b();
            break;

        case 13:
            printf("Führe Aufgabe 1c aus...\n");
            run_aufgabe_1c();
            break;

        case 30:
            printf("Führe Aufgabe 3 aus...\n");
            run_aufgabe_3();
            break;

        case 40:
            printf("Führe Aufgabe 4 aus...\n");
            aufgabe_4();
            break;

        default:
            // Wenn eine ungültige Nummer gewählt wird,
            // geht der Controller in eine Endlosschleife.
            while(1);
            break;
    }
}

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
