#include "inc/tm4c1294ncpdt.h"          // Header of the controller type
#include <stdint.h>                     // Header w. types for the register ..

#define IDLETIME 1000                   // waiting time between transmissions
#define BPS 9600                        // Bits per secound BPS
#define MAXSIZE 10                      // ANF4 Max size of Array

static int wt_3alt = 0;                             // auxillary variable

void config_port_3_alt(void){
    // initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000;       // switch on clock for Port P
    wt_3alt++;                               // delay for stable clock
    // initialize Port P
    GPIO_PORTP_DEN_R |= 0x03;           // enable digital pin function for PP1 & PP0
    GPIO_PORTP_DIR_R |= 0x02;           // set PP1 to output
    GPIO_PORTP_DIR_R &= ~0x01;          // set PP0 to input
    GPIO_PORTP_AFSEL_R |= 0x03;         // switch to alternate pin function PP1 & PP0
    GPIO_PORTP_PCTL_R |= 0x10;          // select alternate pin function PP1->U6Tx
    GPIO_PORTP_PCTL_R |= 0x01;          // select alternate pin function PP0->U6Rx
}

void config_uart_3_alt(void){
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40;          // switch on clock for UART6
    wt_3alt++;                               // delay for stable clock
    UART6_CTL_R &= ~0x01;               // disable UART6 for config
    // initialize bitrate of 115200 bit per second
    UART6_IBRD_R = 16000000/(16*BPS);   // set DIVINT of BRD floor(16 MHz/16*9600 bps)
    UART6_FBRD_R = 44;                  // set DIVFRAC of BRD remaining fraction divider
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

void run_aufgabe_3_alt(void){
    char buffer[MAXSIZE];               // ANF3 Um aus den Daten einen Array zuerstellen
    char c;                             // Um Daten zu empfangen
    int i;

    config_port_3_alt();                      // configuration of Port P
    config_uart_3_alt();                      // configuration of UART6

    while(1){
        // ANF1 und ANF7 neuer Prompt
        send_3_alt(0x0D);                     // send_3_alt the character '\r'
        send_3_alt(0x0A);                     // send_3_alt the character '\n'
        send_3_alt('>');                      // send_3_alt the character '>'

        i = 0;                          // Buffer-Index zuruecksetzen
        while(i < MAXSIZE-1){           // ANF5 minus 1 fuer Platz fuer 0x00
            while(UART6_FR_R & 0x10);   // ANF2 es wird auf Daten gewartet
            c = UART6_DR_R;             // Daten werden Abgelesen um auf EOT zu prüfen
            if (c == 0x04) break;       // EOT im ASCII Code 0x04 wird erkannt und die Schleife beendet             if (i == MAXSIZE-1) break;      // Anforderung 5 MAXSIZE -1
            if (c == 0x0D) break;       // ANF5 0x0D
            buffer[i] = c;              // Zeichen speichern
            i++;
        }
        buffer[i] = 0x00;               // String beenden
        printf("Empfangene Nachricht: %s\n", buffer);          // ANF6
    }
}
