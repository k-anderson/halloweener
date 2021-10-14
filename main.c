/*
 * File:   main.c
 * Author: kanderson
 */

#include <xc.h>
#include <stdbool.h>

// PIC16F627A Configuration Bit Settings
#pragma config FOSC = INTOSCIO  // Oscillator Selection bits (INTOSC oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RA5/MCLR/VPP Pin Function Select bit (RA5/MCLR/VPP pin function is digital input, MCLR internally tied to VDD)
#pragma config BOREN = OFF      // Brown-out Detect Enable bit (BOD disabled)
#pragma config LVP = OFF         // Low-Voltage Programming Enable bit (RB4/PGM pin has PGM function, low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection bit (Data memory code protection off)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

#include "system.h"        /* System funct/params, like osc/peripheral config */

#define FOG 0x01
#define ATRRACTOR 0x02
#define EYES 0x04
#define BLACKLIGHT 0x10
#define STROBE 0x40
#define AUDIO 0x80
#define NONE 0x00

#define LOOP_DELAY 10

int sequence_1[][2] = {
    {11, ATRRACTOR | AUDIO },//11
    
    {255, ATRRACTOR | FOG }, // 2.55s | 2.56s
    {255, ATRRACTOR | FOG }, // 2.55s | 5.11s
    {210, ATRRACTOR | FOG }, // 2s | 7.11s

    {50, ATRRACTOR | EYES}, // .5s | 7.51s
    {10, ATRRACTOR}, // .1s | 7.60s
    {20, ATRRACTOR | EYES}, // .2s | 7.80s
    {10, BLACKLIGHT}, // .1s | 7.90s
    {10, BLACKLIGHT | EYES}, // .1s | 8.00s
    {15, BLACKLIGHT}, // .15s | 8.15s    
    {125, BLACKLIGHT | EYES}, // 1.25s | 9.40s
};

int sequence_2[][2] = {
    {220, BLACKLIGHT}, // 2.2s | 11.60s
    
    {20, EYES}, // .2s | 11.8s
    {10, NONE}, // .1s | 11.9s
    {10, EYES}, // .1s | 12.00s
    {10, NONE}, // .1s | 12.10s
    {20, EYES}, // .2s | 12.30s
    {10, NONE}, // .1s | 12.40s
    {30, EYES}, // .3s | 12.70s
    {70, EYES | STROBE}, // .7s | 13.4s
    {11, STROBE}, // 1.1s | 14.5s
    {20, EYES | STROBE}, // .2s | 14.6s
    {10, STROBE}, // .1s | 14.7s
    {70, EYES | STROBE}, // .2s | 15.00s
};

int sequence_3[][2] = {
    {205, STROBE | EYES}, // 2.55s | 17.55s
    {165, STROBE}, // 1.25s | 18.80s
    
    {30, EYES | STROBE}, // .3s | 19.3s
    {10, STROBE}, // .1s | 19.4s
    {20, BLACKLIGHT | EYES | STROBE}, // .2s | 19.6s
    {10, BLACKLIGHT | STROBE}, // .1s | 19.7s
    {15, BLACKLIGHT | EYES | STROBE}, // .15s | 19.85s
    {10, BLACKLIGHT | STROBE}, // .1s | 19.95s
    {30, BLACKLIGHT | EYES | STROBE}, // .3s | 20.25s
    {10, BLACKLIGHT | STROBE}, // .1s | 20.35s
    {10, BLACKLIGHT | EYES | STROBE}, // .1s | 20.45s
    {10, BLACKLIGHT | STROBE}, // .1s | 20.55s
    {10, BLACKLIGHT | EYES | STROBE}, // .1s | 20.65s  
    {10, BLACKLIGHT | STROBE}, // .1s | 20.75s
    {10, BLACKLIGHT | EYES | STROBE}, // .1s | 20.85s
    {10, BLACKLIGHT | STROBE}, // .1s | 20.95s
    {30, BLACKLIGHT | EYES | STROBE}, // .3s | 21.25s
    
    {255, BLACKLIGHT | EYES}, // 2.55s
    {255, BLACKLIGHT } // 2.55s
};

unsigned int loop_count = 0;
unsigned int sequence_count = 0;

enum {
    IDLE = 0,
    ANIMATION_1,
    ANIMATION_2,
    ANIMATION_3
};

unsigned char state = IDLE;

void switch_state(unsigned char new_state) {
    state = new_state;
    loop_count = 0;
    sequence_count = 0;
}

void __interrupt() interrupt_handler(void) {
    if (INTCONbits.RBIF) {

        if (PORTBbits.RB5 && state == IDLE) {
            switch_state(ANIMATION_1);
        }
        
        INTCONbits.RBIF = 0;
    }
}

void initialize_hw() {
    CMCON = 0x07; // turn comparators off and enable pins for I/O 

    // PORT A
    PORTA = 0xFF; // PORTA initial value
    TRISA = 0x00; // PORTA pins as output (except RA5, output or MCLR only)

    // PORT B
    PORTB = 0x00; // PORTB initial value
    TRISB = 0xFF; // PORTB pins as input
    OPTION_REGbits.nRBPU = 1; // PORTB pull-ups are disabled

    // Interrupts
    INTCONbits.RBIF = 0; // ensure port b flag is not already set
    INTCONbits.RBIE = 1; // enable port b state change interrupts
    OPTION_REGbits.INTEDG = 1; // interrupt on rising edge
    INTCONbits.GIE = 1; // enable general interrupts
    INTCONbits.PEIE = 1; // enable peripheral interrupts    
}

void main(void) {
    initialize_hw();
    
    int sequence_length = 0;

    while (1) {

        switch (state) {
            case IDLE:
                PORTA =~ ATRRACTOR;
                break;
            case ANIMATION_1:
                sequence_length = array_length(sequence_1);
                
                // If we have looped for the number of cycles
                // in this sequence move to the next sequence
                // step
                if (loop_count >= sequence_1[sequence_count][0]) {
                    sequence_count++;
                    loop_count = 0;
                }
                
                // If we have reached the end of the sequence
                // return to idle
                if (sequence_count >= sequence_length) {
                    switch_state(ANIMATION_2);
                    
                // update PORTB with the sequence value
                } else {
                    PORTA =~ sequence_1[sequence_count][1];
                }

                break;
            case ANIMATION_2:
                sequence_length = array_length(sequence_2);
                
                // If we have looped for the number of cycles
                // in this sequence move to the next sequence
                // step
                if (loop_count >= sequence_2[sequence_count][0]) {
                    sequence_count++;
                    loop_count = 0;
                }
                
                // If we have reached the end of the sequence
                // return to idle
                if (sequence_count >= sequence_length) {
                    switch_state(ANIMATION_3);
                    
                // update PORTB with the sequence value
                } else {
                    PORTA =~ sequence_2[sequence_count][1];
                }

                break;
            case ANIMATION_3:
                sequence_length = array_length(sequence_3);
                
                // If we have looped for the number of cycles
                // in this sequence move to the next sequence
                // step
                if (loop_count >= sequence_3[sequence_count][0]) {
                    sequence_count++;
                    loop_count = 0;
                }
                
                // If we have reached the end of the sequence
                // return to idle
                if (sequence_count >= sequence_length) {
                    switch_state(IDLE);
                    
                // update PORTB with the sequence value
                } else { 
                    PORTA =~ sequence_3[sequence_count][1];
                }

                break;                 
        }
        
        __delay_ms(LOOP_DELAY);
        
        loop_count++;
    };
    
    return;
}
