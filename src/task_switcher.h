#ifndef TASK_SWITCHER_H_
#define TASK_SWITCHER_H_

// Eine saubere Methode, um die Aufgaben zu benennen statt Zahlen zu verwenden
typedef enum {
    TASK_1A,
    TASK_1B,
    TASK_1C,
    TASK_3,
    TASK_4,
    TASK_DEBUG,
    NUM_TASKS // Hält die Gesamtzahl der Aufgaben
} TaskID;

// Funktionen, die von main.c aus aufgerufen werden
void buttons_init(void);
void task_switcher_init(void);
void task_switcher_check_and_run(void);

#endif /* TASK_SWITCHER_H_ */
