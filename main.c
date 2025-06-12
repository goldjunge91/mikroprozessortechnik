//===========================================================
// Example Program for the TIVA TM4C1294XL Evaluation Board
// With this file the LEDs D0 to D3 on the eval board will
// blink each one after another. To control the LEDs the
// Ports N and F with Pins PN0, PN1, PF0 and PF4 have to be
// enabled and set as digital output.
//===========================================================
// https://www.incredibuild.com/blog/implicit-vs-explicit-in-programming-key-differences
// https://www.geeksforgeeks.org/embedded-c/
// WHY Volatile
// https://developer.arm.com/documentation/100748/0620/Writing-Optimized-Code/Effect-of-the-volatile-keyword-on-compiler-optimization
// Das volatile-Schlüsselwort zwingt den Compiler dazu,
// die Variable bei jedem Zugriff aus dem Speicher zu lesen, anstatt eine Kopie
// in einem Register zu halten. https://www.ti.com/product/TM4C1294NCPDT

// Include the Header File for controller tm4c1294ncpdt
// #include "inc/tm4c1294ncpdt.h" 			// Header of the
// controller type
#include "src/aufgaben.h"
#include "src/task_switcher.h"
#include "src/tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>
                                                                                                    
// //===========================================================
// // HIER DIE GEWÜNSCHTE AUFGABE AUSWÄHLEN
// //
// // 11 = Aufgabe 1a
// // 12 = Aufgabe 1b
// // 13 = Aufgabe 1c
// // 30 = Aufgabe 3
// // 40 = Aufgabe 4
// //===========================================================
// #define AUFGABE_NUMMER 11

void main(void) {

  buttons_init(); // Taster konfigurieren
                  // Die zentrale Endlosschleife des Programms
  while (1) {
    task_switcher_check_and_run();

  }
}
