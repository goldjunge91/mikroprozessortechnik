#include "task_switcher.h"
#include "aufgaben.h" // Hier deklarieren wir die init/execute Funktionen
#include "tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>

// --- Globale Zustandsvariablen für den Task-Wechsel ---
static const TaskID task_sequence[NUM_TASKS] = {
    TASK_1A, TASK_1B, TASK_1C, TASK_3, TASK_4, TASK_4_OPT, TASK_DEBUG};
static int current_task_index = 0;
static TaskID current_task_id;
static uint8_t task_changed = 1; // Flag, um zu signalisieren, dass eine neue
                                 // Init-Funktion aufgerufen werden muss

void buttons_init(void) {
  SYSCTL_RCGCGPIO_R |= (1 << 8);
  while ((SYSCTL_PRGPIO_R & (1 << 8)) == 0) {
  };
  GPIO_PORTJ_AHB_DIR_R &= ~0x03;
  GPIO_PORTJ_AHB_PUR_R |= 0x03;
  GPIO_PORTJ_AHB_DEN_R |= 0x03;
}
void simple_delay() {
  volatile uint32_t i;
  for (i = 0; i < 50000; i++)
    ;
}
// // Konfiguriert die On-Board-Taster USR_SW1 (PJ0) und USR_SW2 (PJ1)
// void buttons_init(void) {
//     // Takt für Port J aktivieren
//     SYSCTL_RCGCGPIO_R |= (1 << 8);
//     // Warten, bis der Takt stabil ist
//     while((SYSCTL_PRGPIO_R & (1 << 8)) == 0) {};

//     // Pins PJ0 und PJ1 als Eingang konfigurieren
//     GPIO_PORTJ_AHB_DIR_R &= ~0x03;
//     // Interne Pull-Up-Widerstände für PJ0 und PJ1 aktivieren
//     GPIO_PORTJ_AHB_PUR_R |= 0x03;
//     // Digitale Funktion für PJ0 und PJ1 aktivieren
//     GPIO_PORTJ_AHB_DEN_R |= 0x03;
// }

// Führt die Initialisierungsfunktion der aktuellen Aufgabe aus
void run_task_init(TaskID task) {
  switch (task) {
  case TASK_1A:
    printf("Init: Aufgabe 1a - Zeichen 'X', 9600 bps, Format 7E1\n");
    init_aufgabe_1a();
    break;
  case TASK_1B:
    printf("Init: Aufgabe 1b - Zeichen 'a', 38400 bps, Format 8O1\n");
    init_aufgabe_1b();
    break;
  case TASK_1C:
    printf("Init: Aufgabe 1c - Daten 0x3B (';'), 4800 bps, Format 7N2\n");
    init_aufgabe_1c();
    break;
  case TASK_3:
    init_aufgabe_3();
    break;
  case TASK_4:
    init_aufgabe_4();
    break;
  case TASK_4_OPT:
    init_aufgabe_4_optional();
    break;
  case TASK_DEBUG:
    init_debugging();
    break;
  default:
    break;
  }
}

// Führt einen Schritt der aktuellen Aufgabe aus
void run_task_execute(TaskID task) {
  switch (task) {
  case TASK_1A:
    execute_aufgabe_1a('X', 9600);
    break;
  case TASK_1B:
    execute_aufgabe_1b('a', 38400);
    break;
  case TASK_1C:
    execute_aufgabe_1c(';', 4800);
    break;
  case TASK_3:
    // LCRH 0x60 für 8N1
    printf("Führe Aufgabe 3 aus... 0, 9600, LCRH 0x60 für 8N1\n");
    execute_aufgabe_3(0, 9600, 0x60);
    break;
  case TASK_4:
    // LCRH 0x60 für 8N1
    printf("Führe Aufgabe 4 aus...\n");
    execute_aufgabe_4(0, 115200, 0x60);
    break;
  case TASK_4_OPT:
    // LCRH 0x60 für 8N1
    printf("Führe Aufgabe 4_optional aus. 0, 115200, 0x60..\n");
    execute_aufgabe_4(0, 115200, 0x60);
    break;
  // case TASK_DEBUG:
  //     execute_debugging(0, 115200, 0x60);
  //     break;
  default:
    break;
  }
}

// Hauptfunktion, die von main's while-Schleife aufgerufen wird
void task_switcher_check_and_run(void) {
  // Tasten abfragen (aktives Low, da Pull-Up-Widerstände)
  if ((GPIO_PORTJ_AHB_DATA_R & 0x01) ==
      0) {          // USR_SW1 (PJ0) gedrückt -> Nächste Aufgabe
    simple_delay(); // Entprellen
    if ((GPIO_PORTJ_AHB_DATA_R & 0x01) == 0) {
      current_task_index++;
      if (current_task_index >= NUM_TASKS) {
        current_task_index = 0; // Zurück zum Anfang
      }
      task_changed = 1;
      while ((GPIO_PORTJ_AHB_DATA_R & 0x01) == 0) {
      }; // Warten, bis der Knopf losgelassen wird
    }
  } else if ((GPIO_PORTJ_AHB_DATA_R & 0x02) ==
             0) {   // USR_SW2 (PJ1) gedrückt -> Vorherige Aufgabe
    simple_delay(); // Entprellen
    if ((GPIO_PORTJ_AHB_DATA_R & 0x02) == 0) {
      current_task_index--;
      if (current_task_index < 0) {
        current_task_index = NUM_TASKS - 1; // Zum Ende springen
      }
      task_changed = 1;
      while ((GPIO_PORTJ_AHB_DATA_R & 0x02) == 0) {
      }; // Warten, bis der Knopf losgelassen wird
    }
  }

  // Holen der aktuellen Task-ID
  current_task_id = task_sequence[current_task_index];

  // Wenn die Aufgabe gewechselt wurde, rufe die passende init-Funktion auf
  if (task_changed) {
    run_task_init(current_task_id);
    task_changed = 0; // Flag zurücksetzen
  }

  // Führe immer einen Schritt der aktuellen Aufgabe aus
  run_task_execute(current_task_id);
}
