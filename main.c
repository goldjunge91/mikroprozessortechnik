//===========================================================
// Example Program for the TIVA TM4C1294XL Evaluation Board
// With this file the LEDs D0 to D3 on the eval board will
// blink each one after another. To control the LEDs the
// Ports N and F with Pins PN0, PN1, PF0 and PF4 have to be
// enabled and set as digital output.
//===========================================================
// https://www.incredibuild.com/blog/implicit-vs-explicit-in-programming-key-differences
// https://www.geeksforgeeks.org/embedded-c/
// WHY Volatile https://developer.arm.com/documentation/100748/0620/Writing-Optimized-Code/Effect-of-the-volatile-keyword-on-compiler-optimization
// Das volatile-Schlüsselwort zwingt den Compiler dazu, 
// die Variable bei jedem Zugriff aus dem Speicher zu lesen, anstatt eine Kopie in einem Register zu halten.
// https://www.ti.com/product/TM4C1294NCPDT

// Include the Header File for controller tm4c1294ncpdt
// #include "inc/tm4c1294ncpdt.h" 			// Header of the controller type
#include "src/tm4c1294ncpdt.h"
#include "src/aufgaben.h"
#include <stdint.h>
#include <stdio.h>
#include "src/task_switcher.h"

//===========================================================
// HIER DIE GEWÜNSCHTE AUFGABE AUSWÄHLEN
//
// 11 = Aufgabe 1a
// 12 = Aufgabe 1b
// 13 = Aufgabe 1c
// 30 = Aufgabe 3
// 40 = Aufgabe 4
//===========================================================
#define AUFGABE_NUMMER 11

void main(void) {

    buttons_init(); // Taster konfigurieren
 // Die zentrale Endlosschleife des Programms
    while(1) {
        task_switcher_check_and_run();

    // switch (AUFGABE_NUMMER) {
    //     case 11:
    //         printf("Führe Aufgabe 1a aus...\n");
    //         run_aufgabe_1a('A', 9600);
    //         break;

    //     case 12:
    //         printf("Führe Aufgabe 1b aus...\n");
    //         // run_aufgabe_1b();
    //         run_aufgabe_1b('A', 38400);
    //         break;

    //     case 13:
    //         printf("Führe Aufgabe 1c aus...\n");
    //         // run_aufgabe_1c(zeichen: ";", baudrate: 48000);
    //         run_aufgabe_1c('A', 4800);
    //         break;

    //     case 30:
    //         printf("Führe Aufgabe 3 aus...\n");
    //         // run_aufgabe_3();
    //         run_aufgabe_3_alt();
    //         break;

    //     case 40:
    //         printf("Führe Aufgabe 4 aus...\n");
    //         aufgabe_4();
    //         break;

    //     default:
    //         // Wenn eine ungültige Nummer gewählt wird,
    //         // geht der Controller in eine Endlosschleife.
    //         while(1);
    //         break;
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
