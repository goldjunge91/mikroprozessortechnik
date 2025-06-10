//===========================================================
//  Example Program for the TIVA TM4C1294XL Evaluation Board
//  With this file the LEDs D0 to D3 on the eval board will
//  blink each one after another. To control the LEDs the
//  Ports N and F with Pins PN0, PN1, PF0 and PF4 have to be
//  enabled and set as digital output.
//===========================================================
// Include the Header File for controller tm4c1294ncpdt

#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>

#define IDLETIME 1000
#define MAXSIZE 10
#define PORTM 0x00000800
static int wt_l4 = 0;
static int a = 0;

void config_port_4(void)
{
    // initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000; // switch on clock for Port P
    SYSCTL_RCGCGPIO_R |= PORTM;   // switch on clock for Port M
    wt_l4++;
    // initialize Port P U6Tx
    GPIO_PORTP_DEN_R |= 0x2;   // enable digital pin function for PP1
    GPIO_PORTP_DIR_R |= 0x2;   // set PP1 to output
    GPIO_PORTP_AFSEL_R |= 0x2; // switch to alternate pin function PP1
    GPIO_PORTP_PCTL_R |= 0x10; // select alternate pin function PP1->U6Tx
    // initialize Port P U6Rx
    GPIO_PORTP_DEN_R |= 0x1;   // enable digital pin function for PP0
    GPIO_PORTP_DIR_R |= ~0x1;  // set PP0 to input
    GPIO_PORTP_AFSEL_R |= 0x1; // switch to alternate pin function PP0
    GPIO_PORTP_PCTL_R |= 0x1;  // select alternate pin function PP1->U6Rx
    // initialize Port M
    GPIO_PORTM_DEN_R |= 0x0F; // enable digital pin function for PP1
    GPIO_PORTM_DIR_R |= 0x0F; // set PP1 to output
}
void config_uart_4(void)
{
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40; // switch on clock for UART6
    wt_l4++;                      // delay for stable clock
    UART6_CTL_R &= ~0x01;      // disable UART6 for config
                               // initialize bitrate of 115200 bit per second
    UART6_IBRD_R = 8;          // set DIVINT of BRD floor(16 MHz/16*115200 bps)
    UART6_FBRD_R = 44;         // set DIVFRAC of BRD remaining fraction divider
    UART6_LCRH_R = 0x00000060; // serial format 8N1
    UART6_CTL_R |= 0x0301;     // UART transmit on/reciever on and UART enable
}
void idle_4()
{ // simple wait for idle_4 state
    int i = 0;
    for (i = IDLETIME; i > 0; i--)
        ; // count down loop for waiting
}
void labor_aufgabe_4(void)
{
    char c, buffer[MAXSIZE]; // Char c um Datenarray zu empfangen, buffer um Array zu erstellen
    int i = 0;               // mit Maximallänger
    config_port_4();         // configuration of Port P
    config_uart_4();         // configuration of UART6
    while (1)
    {
        while ((UART6_FR_R & 0x20) != 0)
            ;              // till transmit FIFO not full
        UART6_DR_R = '\r'; // send the character '\r'
        idle_4();          // idle_4 time
        UART6_DR_R = '\n'; // send the character '\n'
        idle_4();
        UART6_DR_R = '>'; // send the character '>'
        idle_4();
        while (i < MAXSIZE)
        {
            while (UART6_FR_R & 0x10)
                ;           // es wird auf Daten gewartet
            c = UART6_DR_R; // Daten werden Abgelesen um auf EOT zu prüfen
            buffer[i] = c;  // Ausgabe c
            if (buffer[0] == 'l' && buffer[1] == 'e' && buffer[2] == 'd')
            {
                if (buffer[3] == '+')
                {
                    switch (buffer[4])
                    {
                    case '1':
                        GPIO_PORTM_DATA_R |= 0x1;
                        break;
                    case '2':
                        GPIO_PORTM_DATA_R |= 0x2;
                        break;
                    case '3':
                        GPIO_PORTM_DATA_R |= 0x4;
                        break;
                    case '4':
                        GPIO_PORTM_DATA_R |= 0x8;
                        break;
                    }
                }
                else if (buffer[3] == '-')
                {
                    switch (buffer[4])
                    {
                    case '1':
                        GPIO_PORTM_DATA_R &= 0xE;
                        break;
                    case '2':
                        GPIO_PORTM_DATA_R &= 0xD;
                        break;
                    case '3':
                        GPIO_PORTM_DATA_R &= 0xB;
                        break;
                    case '4':
                        GPIO_PORTM_DATA_R &= 0x7;
                        break;
                    }
                }
            }
            if (c == 0x04)
                break; // EOT im ASCII Code 0x04 wird erkannt und die Schleife beendet
            if (i == MAXSIZE - 1)
                break; // Anforderung 5 MAXSIZE -1
            if (c == 0x0D)
                break; // Anforderung 5 0x0D
            i++;
        }
        buffer[i] = 0x00; // letzter slot des Arrays wir mit "\0" beschrieben
        i = 0;
        printf("\nGesendete Nachricht: %s\n", buffer);
    }
}
