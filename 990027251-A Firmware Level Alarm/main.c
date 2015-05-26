/*
 * File:   main.c
 * Author: Noah Bergman
 *
 *
 * The purpose of this program is to sound an alarm/light when the
 * desired level is reached in a fiberglass resin tank.
 *
 * Cap Sensor Output High when level is low - Pull Down when Wet
 *
 *
 *
 * Created on April 16, 2015, 11:56 AM
 */
#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "util.h"
/*
 *
 */
// CONFIG
#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config CP = OFF         // FLASH Program Memory Code Protection bits (Code protection off)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF         // SET OFF FOR STROBE ENABLE Low Voltage In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EE Memory Code Protection (Code Protection off)
#pragma config WRT = ON         // FLASH Program Memory Write Enable (Unprotected program memory may be written to by EECON control)


#define _bv(bit) (1<<bit)

//LEDs
#define ALMLED (1<<3) //Alarm Indicator
#define POWLED (1<<2) //Power / Error Indicator
#define LEDPORT PORTC
#define LEDTRIS TRISC

//Light and Siren
#define ALMOUT (1<<2)   //SIREN
#define LGTOUT (1<<3)   //STROBE
#define ALMPORT PORTB
#define ALMTRIS TRISB

//Sensor Inputs
#define LVLTWO (1<<5)
#define LVLONE (1<<4)
#define SNSPORT PORTB
#define SNSTRIS TRISB

#define ADCPORT PORTA
#define ADCPIN (1<<0)   //RA0

//Delays for Timers
const int FILTERTMR_ISFULL = 50;   //Counter going from Full to Empty
const int FILTERTMR_ISEMPTY = 50;   //Counter going from Empty to Full
//const int ALARMTMR = 50;
volatile int ALARMTMR = 100;            //Alarm delay - ADC Result
volatile int counter2 = 0;
volatile int counter1 = 0;
volatile int counter0 = 0;

//0 - Rest
//1 - Alarm is Ready
//2 - Single Level Alarm
//3 - Double Alarm Count Fast

volatile uint8_t alarmState = 0;
volatile uint8_t lvlOneState = 0;
volatile uint8_t lvlTwoState = 0;
volatile int adcResult =0;


void interrupt ISR(){

   // TODO: Fix patterns for ALMLED Blinking

    //Timer 1 Overflow
    if(TMR1IE && TMR1IF){
        LEDPORT ^= ALMLED;
        if(alarmState == 2){
            counter1 ++;
        }
        if(alarmState == 3){
            counter1 += 2;  //Double Time
        }
        TMR1IF = 0;
    }

    //Timer 2 Overflow - Filter for LVL Two
    if(TMR2IF && TMR2IE){
        counter2 ++;
        LEDPORT ^= ALMLED;
        TMR2IF = 0;
    }

    //ADC Interrupt Flag
    if(ADIF && ADIE){
       adcResult = (ADRESH <<8) | (ADRESL);
       ALARMTMR = adcResult >> 2;
       ADIF = 0;
    }

//    //Timer 0 Overflow - Filter for LVL One
//    if(TMR0IF && TMR0IE){
//        counter0 ++;
//        LEDPORT^= ALMLED;
//        TMR0IF = 0;
//
//    }
}




int main(int argc, char** argv) {
    uint8_t sensorRead =0;
    uint8_t lvlOneRead = 0;
    uint8_t lvlTwoRead = 0;

    int i = 0;


    LEDTRIS &= ~(POWLED|ALMLED);
    SNSTRIS |= LVLONE|LVLTWO;
    ALMTRIS &= ~(LGTOUT|ALMOUT);
    ADCPORT |= ADCPIN;

    timer1_init();
    timer2_init();
    adc_init_CH0();

    LEDPORT |= POWLED;
    ALMPORT &= ~(ALMOUT|LGTOUT);


   while(1){

        sensorRead = SNSPORT & (LVLTWO | LVLONE);
        lvlOneRead = (sensorRead & LVLONE);
        lvlTwoRead = (sensorRead & LVLTWO);

 /****************Second Level State System*************************************/
//        if(lvlOneState == 0){                           //Initial State - Wait for tub to be empty
//
//            // ALMPORT &= ~(ALMOUT|LGTOUT);
//            if((lvlOneRead > 0) && (counter0 < 2)){            //Tub Empty - Rising Edge
//                lvlOneState = 1;   //Start Filter to go Down
//                timer0_start();
//
//            }else {                                     //Not Empty Yet - Stop Timer and Counter
//                timer0_stop();
//                counter0 = 0;
//                LEDPORT &= ~(SNSLED | ALMLED);
//            }
//
//        }else if(lvlOneState == 1){                    //Filter Transistion to Empty - Flashing SNSLED
//            //LEDPORT |= ALMLED;      //Debug
//            if(lvlOneRead == 0){                        //Full Tub - Restart Filter
//                counter0 = 0;
//                timer0_stop();
//                lvlOneState = 0;
//
//            }else if(lvlOneRead > 0 && counter0 >= FILTERTMR_ISFULL){  //Empty Tub - Stop Timer, Next State
//                timer0_stop();
//                counter0 = 0;
//                lvlOneState = 2;
//            }
//
//        }else if(lvlTwoState == 2){                    //Filter Transition to Full - Flashing SNSLED
//                                                        //
//           // LEDPORT |= ALMLED;       //Debug
//            if(lvlOneRead == 0 && counter0 == 0){     //Full - Rising Edge Start Filter
//                timer0_start();
//                counter0 = 1;
//
//
//            }else if(lvlOneRead > 0){  //Empty - Falling Edge Stop Filter
//                timer0_stop();
//                counter0 = 0;
//                LEDPORT &= ~SNSLED;
//                //Sleep ??
//
//            }else if(lvlOneRead == 0 && counter0 > FILTERTMR_ISEMPTY){  //Complete Fill - Start Alarm
//                LEDPORT |= SNSLED;
//                lvlOneState = 3;
//                alarmState = 1;
//                timer0_stop();
//                counter0 = 0;
//            }else{
//
//            }
//        }else if(lvlOneState == 3){                             //Alarm Started
//            //ALMPORT = 0x30;       //Debug
//                lvlOneState = 4;    //Get out of this state
//                alarmState ++;      //Increment Priority Level for multiple sensors
//        }

/***************************Level Two State System****************************/

        if(lvlTwoState == 0){                           //Initial State - Wait for tub to be empty

            if((lvlTwoRead > 0) && (counter2 < 1)){            //Tub Empty - Rising Edge
                lvlTwoState = 1;   //Start Filter to go Down
                timer2_start();

            }else {                                     //Not Empty Yet - Stop Timer and Counter
                timer2_stop();
                counter2 = 0;
                LEDPORT &= ~ALMLED;
            }

        }else if(lvlTwoState == 1){                    //Filter Transistion to Empty - Flashing SNSLED
            //LEDPORT |= ALMLED;      //Debug
            if(lvlTwoRead == 0){                        //Full Tub - Restart Filter
                counter2 = 0;
                timer2_stop();
                lvlTwoState = 0;

            }else if(lvlTwoRead > 0 && counter2 >= FILTERTMR_ISFULL){  //Empty Tub - Stop Timer, Next State
                timer2_stop();
                counter2 = 0;
                lvlTwoState = 2;
            }

        }else if(lvlTwoState == 2){                    //Filter Transition to Full - Flashing SNSLED
                                                        //
           // LEDPORT |= ALMLED;       //Debug
            if(lvlTwoRead == 0 && counter2 == 0){     //Full - Rising Edge Start Filter
                timer2_start();
                counter2 = 1;


            }else if(lvlTwoRead > 0){  //Empty - Falling Edge Stop Filter
                timer2_stop();
                counter2 = 0;
                LEDPORT &= ~ALMLED;     //Turn off ALMLED

            }else if(lvlTwoRead == 0 && counter2 > FILTERTMR_ISEMPTY){  //Complete Fill - Start Alarm
                //LEDPORT |= ALMLED;    //Debug
                lvlTwoState = 3;
                alarmState = 1;
                timer2_stop();
                counter2 = 0;
            }else{

            }
        }else if(lvlTwoState == 3){                             //Alarm Started
            //ALMPORT |= LGTOUT;       //Debug
            lvlTwoState = 4;    //Get out of this state
            alarmState ++;      //Increment Priority Level for multiple sensors
        }


/********************ALARM ON***************************************************/
        if(alarmState >= 1){                                   //Alarm On - Flash ALMLED, SNSLED ON
            //TODO: Allow Toggling for second filter
            if(counter1 == 0){      //Timer Started - LEDS doing their thing
                ALMPORT |= LGTOUT;
                LEDPORT |= ALMLED;
                adc_start();
                timer1_start();
                counter1 = 1;

            }else if(counter1 >= ALARMTMR){ //Alarm Timer Reached - Engage Siren
                ALMPORT |= ALMOUT |LGTOUT;      
                timer1_stop();
                LEDPORT |= ALMLED;
                counter1 = 1;   //Don't allow timer1 to restart
            }
        }


    }

    while(1){
        LEDPORT^=POWLED;
        for(i=0;i<10000;i++); //NOT A GOOD SIGNAL!! ERROR!

    }
    return (EXIT_SUCCESS);
}

