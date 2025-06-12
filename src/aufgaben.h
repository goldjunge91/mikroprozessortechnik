#ifndef AUFGABEN_H_
#define AUFGABEN_H_
#include "tm4c1294ncpdt.h"
#include <stdint.h>


// ############# wird nicht benötigt aber behalten bis alles funktioniert.
void run_aufgabe_1a(char zeichen, uint32_t baudrate);
void run_aufgabe_1b(char zeichen, uint32_t baudrate);
void run_aufgabe_1c(char zeichen, uint32_t baudrate);

void run_aufgabe_3(char zeichen, uint32_t baudrate);
void aufgabe_4(uint32_t baudrate);
void testing_uart(void);
// ############# wird nicht benötigt aber behalten bis alles funktioniert.

// ############# neue implementierung
// Aufgabe 1a
void init_aufgabe_1a(void);
void execute_aufgabe_1a(char zeichen, uint32_t baudrate);
// Aufgabe 1a
void init_aufgabe_1b(void);
void execute_aufgabe_1b(char zeichen, uint32_t baudrate);
// Aufgabe 1c
void init_aufgabe_1c(void);
void execute_aufgabe_1c(char zeichen, uint32_t baudrate);

// Aufgabe 3
void init_aufgabe_3(void);
void execute_aufgabe_3(char zeichen, uint32_t baudrate, uint32_t lcrh_setting);

// Aufgabe 4
void init_aufgabe_4(void);
void execute_aufgabe_4(char zeichen, uint32_t baudrate, uint32_t lcrh_setting);

// Aufgabe 4
void aufgabe_4(uint32_t baudrate);
void init_aufgabe_4_optional(void);
void execute_aufgabe_4_optional(char zeichen, uint32_t baudrate, uint32_t lcrh_setting);

// Debugging
void init_debugging(void);
void execute_debugging(char zeichen, uint32_t baudrate, uint32_t lcrh_setting);
#endif /* AUFGABEN_H_ */

