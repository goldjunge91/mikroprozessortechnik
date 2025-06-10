//===========================================================
// Test program UART6 TX 8/N/1 @ 115200 bit via Port PP1
// LTL 17.5.2020 / mod. V0.2 - V0.4 RMS 1.6.2023
//==========================================================
#include "inc/tm4c1294ncpdt.h" // Header of the controller type
#include <stdint.h>            // Header w. types for the register ..
#define IDLETIME 1000          // waiting time between transmissions
#define BPS 9600               // Bits per secound BPS
#define MAXSIZE 10             // Max size of Array

int wt_3 = 0;                    // auxillary variable

void config_port_3(void)
{
    // initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000; // switch on clock for Port P
    wt_3++;                         // delay for stable clock
    // initialize Port P
    GPIO_PORTP_DEN_R |= 0x03;   // enable digital pin function for PP1 & PP0
    GPIO_PORTP_DIR_R |= 0x02;   // set PP1 to output
    GPIO_PORTP_DIR_R |= 0x00;   // set PP0 to input
    GPIO_PORTP_AFSEL_R |= 0x03; // switch to alternate pin function PP1 & PP0
    GPIO_PORTP_PCTL_R |= 0x10;  // select alternate pin function PP1->U6Tx
    GPIO_PORTP_PCTL_R |= 0x01;  // select alternate pin function PP0->U6Rx
}
void config_uart_3(void)
{
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40; // switch on clock for UART6
    wt_3++;                      // delay for stable clock
    UART6_CTL_R &= ~0x01;      // disable UART6 for config
    // initialize bitrate of 115200 bit per second
    UART6_IBRD_R = 16000000 / (16 * BPS); // set DIVINT of BRD floor(16 MHz/16*9600 bps)
    UART6_FBRD_R = 44;                    // set DIVFRAC of BRD remaining fraction divider
    UART6_LCRH_R = 0x00000060;            // serial format 8N1
    UART6_CTL_R |= 0x0301;                // UART transmit and receive on and UART enable
}
void idle_3()
{ // simple wait for idle state
    int i;
    for (i = IDLETIME; i > 0; i--)
        ; // count down loop for waiting
}
void run_aufgabe_3(void)
{
    char buffer[MAXSIZE]; // Um aus den Daten einen Array zuerstellen
    char c;               // Um Daten zu empfangen
    int i;
    config_port_3(); // configuration of Port P
    config_uart_3(); // configuration of UART6
    while (1)
    {
        while ((UART6_FR_R & 0x10) != 0)
            ;              // till transmit FIFO not full
        UART6_DR_R = 0x0D; // send the character '\r'
        idle_3();          // idle_3 time
        UART6_DR_R = 0x0A; // send the character '\n'
        idle_3();          // idle_3 time
        UART6_DR_R = '>';  // send the character '>'
        idle_3();
        while (i < MAXSIZE)
        {
            while (UART6_FR_R & 0x10)
                ;           // es wird auf Daten gewartet
            c = UART6_DR_R; // Daten werden Abgelesen um auf EOT zu prüfen
            if (c == 0x04)
                break; // EOT im ASCII Code 0x04 wird erkannt und die Schleife beendet
            if (i == MAXSIZE - 1)
                break; // Anforderung 5 MAXSIZE -1
            if (c == 0x0D)
                break;     // Anforderung 5 0x0D
            buffer[i] = c; // Ausgabe c
            i++;
        }
        buffer[i] = 0x00;
    }
}
