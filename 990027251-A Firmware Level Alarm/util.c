#include <xc.h>
#include <stdio.h>
#include <stdint.h>
#include "util.h"



//Setup Timer 1 - 16bit
void timer1_init(){
    //Prescalar 1:8 - 0x30
    //T1OSCEN - OSCEnable - 0x08
    //T1SYNC
    //TMR1CS - Internal Clock - 0
    //TMR1ON - Enable When Ready
    T1CONbits.T1CKPS = 0x03;    //Prescalar /8
    T1CONbits.TMR1CS = 0;   //Internal Clock Select
}

//Start Timer 1
void timer1_start(){
    T1CONbits.T1OSCEN = 1;  //Enable Osc
    T1CONbits.TMR1ON = 1;   //Enable Timer
    PIE1bits.TMR1IE = 1;    //Enable Interrupt
    INTCONbits.GIE = 1;     //Global
    INTCONbits.PEIE = 1;    //Preipheral

}

//Stop Timer 1
void timer1_stop(){
    T1CONbits.TMR1ON = 0;
    T1CONbits.T1OSCEN =0;
    PIE1bits.TMR1IE = 0;
}


//Setup Timer2 - 8bit
void timer2_init(){
    T2CONbits.T2CKPS = 0x02;    //Prescale /4
    T2CONbits.TOUTPS = 0x0F;    //Postscale /16
}

void timer2_start(){
    PIE1bits.TMR2IE = 1;
    T2CONbits.TMR2ON = 1;
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}

void timer2_stop(){
    T2CONbits.TMR2ON = 0;
    PIE1bits.TMR2IE = 0;
}

//Initialize Time 0
void timer0_init(){
    OPTION_REGbits.T0CS = 0;    //Internal Clock
    OPTION_REGbits.PSA = 0;     //Prescale Select Timer0 vs. WDTMR
    OPTION_REGbits.PS = 0x04;   //Prescale /16
}

//Enable timer 0 - Enable the interrupt
void timer0_start(){
    INTCONbits.TMR0IE = 1;  //Enable Interrupt
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;

}
//Stop Timer 0 - Disable the interrupt
void timer0_stop(){
    //No timer disable
    INTCONbits.TMR0IE = 0;
}

//Init ADC for Channel 0 - RA0
void adc_init_CH0(){
    ADCON0bits.ADCS = 0x00;     //Clock /2
    ADCON0bits.CHS = 0x00;      //Channel 0 - RA0

    ADCON1bits.ADFM = 1;        //Right Justified
    ADCON1bits.PCFG = 0x00;     //Channel 0 - VDD

}

//Start ADC Conversion
void adc_start(){
    ADCON0bits.ADON =1;         //AD ON
    ADCON0bits.GO_nDONE = 1;    //AD Start
    PIE1bits.ADIE =1;           //Enable Interrupt
    INTCONbits.GIE =1;
    INTCONbits.PEIE = 1;
}

void blinkLed(uint8_t stateOne, uint8_t stateTwo, uint8_t almState, uint8_t *blinkState){
    if(stateOne == TRANSITION_TO_EMPTY || stateOne == TRANSITION_TO_FULL || stateTwo == TRANSITION_TO_EMPTY || stateTwo == TRANSITION_TO_FULL){
        *blinkState |= 0x01;    //Blink Fast
    }else if(almState >= 1 && almState < 3){
        *blinkState |= 0x04;
    }else if (almState >= 3){
        *blinkState |= 0x08;
        LEDPORT |= ALMLED;
    }else{
        *blinkState = 0;
        LEDPORT &= ~ALMLED;
    }

}
