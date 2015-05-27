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




//Delays for Timers
const int FILTERTMR_ISFULL = 50;   //Counter going from Full to Empty
const int FILTERTMR_ISEMPTY = 50;   //Counter going from Empty to Full
//const int ALARMTMR = 50;
volatile int alarmTimer = 100;            //Alarm delay - ADC Result
volatile int counter1 = 0;
volatile int counter2 = 0;
volatile int blinkCounter = 0;
volatile int almCounter = 0;


//0 - Rest
//1 - Alarm is Ready
//2 - Single Level Alarm
//3 - Double Alarm Count Fast

volatile uint8_t alarmState = 0;
volatile uint8_t lvlOneState = 0;
volatile uint8_t lvlTwoState = 0;
volatile int adcResult = 100;

//1 - Level One Filter - Blink Fast
//2 - Level Two Filter - Blink Fast
//4 - Alarm On - Blink Slow
volatile uint8_t blinkState = 0; //Determines the blinking pattern of ALMLED


void interrupt ISR(){

   // TODO: Fix patterns for ALMLED Blinking

    //Timer 0 Overflow - Filter for LVL One
    if(TMR0IF && TMR0IE){
        
        if((blinkState == 1) && (blinkCounter % 5 == 0)){
            LEDPORT ^= ALMLED;
        }else if((blinkState == 4) && (blinkCounter % 10 == 0)){
            LEDPORT ^= ALMLED; //Toggle if just Level One
        }
        blinkCounter ++;
        TMR0IF = 0;
    }
    
    //Timer 1 Overflow
    if(TMR1IE && TMR1IF){
        LEDPORT ^= ALMLED;  //Toggle Only when just counting to alarm

        if(alarmState == 2){
            almCounter ++;
        }
        if(alarmState == 3){
            almCounter += 2;  //Double Time
        }
        TMR1IF = 0;
    }

    //Timer 2 Overflow - Filter Counter for both Sensors
    if(TMR2IF && TMR2IE){
        if(lvlOneState == 1 || lvlOneState == 3){   //Transition States
            counter1++;
        }
        
        if(lvlTwoState == 1 || lvlTwoState == 3){   //Transition States
            counter2++;
        }
        TMR2IF = 0;
    }

    //ADC Interrupt Flag
    if(ADIF && ADIE){
       adcResult = (ADRESH <<8) | (ADRESL);
       //alarmTimer = adcResult >> 2;
       alarmTimer = 20; //Debug
       ADIF = 0;
    }
    


}




int main() {
    uint8_t sensorRead =0;
    uint8_t lvlOneRead = 0;
    uint8_t lvlTwoRead = 0;
    uint8_t adcDelay = 0;
    int i = 0;

    ADCTRIS |= ADCPIN;
    SNSTRIS |= LVLONE|LVLTWO;
    LEDTRIS &= ~(POWLED|ALMLED);
    ALMTRIS &= ~(LGTOUT|ALMOUT);
    

    timer0_init();
    timer1_init();
    timer2_init();
    adc_init_CH0();

    LEDPORT |= POWLED;
    ALMPORT &= ~(ALMOUT|LGTOUT);

    timer0_start(); //Blinking Timer
    timer2_start(); //Start Timer Counting  TODO: Shutoff if Both Sensors dont use
    
   while(1){
       
       //Refresh the ADC value every now and then
       if(adcDelay >= 100 && ADCON0bits.GO_nDONE == 0){
           adc_start();
           adcDelay = 0;
       }else {adcDelay ++;}


        sensorRead = SNSPORT & (LVLTWO | LVLONE);
        lvlOneRead = (sensorRead & LVLONE);
        lvlTwoRead = (sensorRead & LVLTWO);

 /****************Second Level State System*************************************/
    if(lvlOneState == 0){                           //Initial State - Wait for tub to be empty
            if(lvlOneRead > 0){             //Tub Empty - Rising Edge
                lvlOneState = 1;   //Start Filter to go Down
            }else {                         //Not Empty Yet - Stop Timer and Counter
                counter1 = 0;
            }


        }else if(lvlOneState == 1){                    //Filter Transistion to Empty - Flashing SNSLED
            //LEDPORT |= ALMLED;      //Debug
            if(lvlOneRead == 0){      //Full Tub - Restart Filter
                counter1 = 0;
                lvlOneState = 0;

            }else if(lvlOneRead > 0 && counter1 >= FILTERTMR_ISFULL){  //Empty Tub - Stop Timer, Next State
                counter1 = 0;
                lvlOneState = 2;
                //LEDPORT &= ~ALMLED;
            }

        }else if(lvlOneState == 2){                                    //Idle While Empty
           // LEDPORT |= ALMLED;       //Debug
            if(lvlOneRead == 0 && counter2 == 0){     //Full - Rising Edge Start Filter
                counter1 = 1;

            }else{
                //TODO: Sleep
            }

        }else if(lvlOneState == 3){                             //Transition to Full

            if(lvlOneRead > 0){                //Temp Full - Back to Idle
                counter1 = 0;
                lvlOneState = 2;
                //LEDPORT &= ~ALMLED;

            }else if(lvlOneRead == 0 && counter1 > FILTERTMR_ISEMPTY){
                //LEDPORT |= ALMLED;    //Debug
                counter1 = 0;
                lvlOneState = 4;
                alarmState = 1;
            }


        }else if(lvlOneState == 4){                             //Alarm Started
            //ALMPORT |= LGTOUT;       //Debug
            lvlOneState = 5;    //Get out of this state
            alarmState ++;      //Increment Priority Level for multiple sensors
        }

/***************************Level Two State System****************************/

        if(lvlTwoState == 0){                           //Initial State - Wait for tub to be empty
            if(lvlTwoRead > 0){             //Tub Empty - Rising Edge
                lvlTwoState = 1;   //Start Filter to go Down
            }else {                         //Not Empty Yet - Stop Timer and Counter
                counter2 = 0;
            }


        }else if(lvlTwoState == 1){                    //Filter Transistion to Empty - Flashing SNSLED
            //LEDPORT |= ALMLED;      //Debug
            if(lvlTwoRead == 0){      //Full Tub - Restart Filter
                counter2 = 0;
                lvlTwoState = 0;

            }else if(lvlTwoRead > 0 && counter2 >= FILTERTMR_ISFULL){  //Empty Tub - Stop Timer, Next State
                counter2 = 0;
                lvlTwoState = 2;
                //LEDPORT &= ~ALMLED;
            }

        }else if(lvlTwoState == 2){                                    //Idle While Empty
           // LEDPORT |= ALMLED;       //Debug
            if(lvlTwoRead == 0 && counter2 == 0){     //Full - Rising Edge Start Filter
                counter2 = 1;

            }else{
                //TODO: Sleep
            }

        }else if(lvlTwoState == 3){                             //Transition to Full

            if(lvlTwoRead > 0){                //Temp Full - Back to Idle
                counter2 = 0;
                lvlTwoState = 2;
                //LEDPORT &= ~ALMLED;

            }else if(lvlTwoRead == 0 && counter2 > FILTERTMR_ISEMPTY){
                //LEDPORT |= ALMLED;    //Debug
                counter2 = 0;
                lvlTwoState = 4;
                alarmState = 1;
            }


        }else if(lvlTwoState == 4){                             //Alarm Started
            //ALMPORT |= LGTOUT;       //Debug
            lvlTwoState = 5;    //Get out of this state
            alarmState ++;      //Increment Priority Level for multiple sensors
        }


/********************ALARM ON***************************************************/
        if(alarmState >= 1){                                   //Alarm On - Flash ALMLED, SNSLED ON
            //TODO: Allow Toggling for second filter
            if(almCounter == 0){      //Timer Started - LEDS doing their thing
                ALMPORT |= LGTOUT;
                LEDPORT |= ALMLED;
                adc_start();
                timer1_start();
                almCounter = 1;

            }else if(almCounter > alarmTimer){ //Alarm Timer Reached - Engage Siren
                ALMPORT |= ALMOUT |LGTOUT;      
                timer1_stop();
                LEDPORT |= ALMLED;
                almCounter = 1;   //Don't allow timer1 to restart
            }
        }


        blinkLed(lvlOneState, lvlTwoState, alarmState, &blinkState);

    }


    //NOT A GOOD SIGNAL!! ERROR!
    while(1){
        LEDPORT^=POWLED;
        for(i=0;i<10000;i++); 

    }
    return (EXIT_SUCCESS);
}

