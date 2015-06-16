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
//volatile levelStates lvlOneState = INITIAL_STATE;
//volatile levelStates lvlTwoState = INITIAL_STATE;
volatile int adcResult = 100;

levelSensor_t levelSensor[2];

//1 - Level One Filter - Blink Fast
//2 - Level Two Filter - Blink Fast
//4 - Alarm On - Blink Slow
volatile uint8_t blinkState = 0; //Determines the blinking pattern of ALMLED


void interrupt ISR(){

   // TODO: Fix patterns for ALMLED Blinking

    //Timer 0 Overflow - Filter for LVL One
    if(TMR0IF && TMR0IE){
        if((blinkState & (0x01)== 1) && (blinkCounter % 5 == 0)){
            LEDPORT ^= ALMLED;
        }else if((blinkState == 4) && (blinkCounter % 20 == 0)){
            LEDPORT ^= ALMLED; //Toggle if alm
        }
        blinkCounter ++;
        TMR0IF = 0;
    }
    
    //Timer 1 Overflow
    if(TMR1IE && TMR1IF){


        if(alarmState == 1){
            almCounter ++;
        }else if(alarmState == 2){
            almCounter += 3;  //Double Time
        }else if(alarmState >= 3){
            //Turn Off Alarm -- Completed in Alarm State 
        }

        TMR1IF = 0;
    }

    //Timer 2 Overflow - Filter Counter for both Sensors
    if(TMR2IF && TMR2IE){
        if(levelSensor[0].levelState == TRANSITION_TO_EMPTY || levelSensor[0].levelState == TRANSITION_TO_FULL){   //Transition States
            counter1++;
        }
        
        if(levelSensor[1].levelState == TRANSITION_TO_EMPTY || levelSensor[1].levelState == TRANSITION_TO_FULL){   //Transition States
            counter2++;
        }
        TMR2IF = 0;
    }

    //ADC Interrupt Flag
    if(ADIF && ADIE){
       adcResult = (ADRESH <<8) | (ADRESL);
       alarmTimer = adcResult >> 2;
       //alarmTimer = 20; //Debug
       ADIF = 0;
    }
    


}




int main() {

    uint8_t sensorRead =0;
    
    levelSensor_t levelSensor[2];
    sensorRead = SNSPORT & (LVLTWO | LVLONE);
    levelSensor[0].sensorRead = (sensorRead & LVLONE);
    levelSensor[1].sensorRead = (sensorRead & LVLTWO);

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

    
   while(1)
   {

//Not Used at This Time - ADC Refreshes at the strart of alarm cycle
       //Refresh the ADC value every now and then
//       if(adcDelay >= 1000 && ADCON0bits.GO_nDONE == 0){
//           adc_start();
//           adcDelay = 0;
//       }else {adcDelay ++;}

       
 /****************Level Zero State System*************************************/
        switch(levelSensor[0].levelState){
            case INITIAL_STATE :                           //Initial State - Wait for tub to be empty
                if(levelSensor[0].sensorRead > 0){             //Tub Empty - Rising Edge
                    levelSensor[0].levelState = TRANSITION_TO_EMPTY;   //Start Filter to go Down
                }else {                         //Not Empty Yet - Stop Timer and Counter
                    counter1 = 0;
                }
                break;
            
            case TRANSITION_TO_EMPTY :                   //Filter Transistion to Empty - Flashing SNSLED
                //LEDPORT |= ALMLED;      //Debug
                if(levelSensor[0].sensorRead == 0){      //Full Tub - Restart Filter

                    counter1 = 0;
                    levelSensor[0].levelState = INITIAL_STATE;

                }else if(levelSensor[0].sensorRead > 0 && counter1 >= FILTERTMR_ISFULL){  //Empty Tub - Stop Timer, Next State
                    counter1 = 0;
                    levelSensor[0].levelState = EMPTY;

                }
                break;
                
            case EMPTY:                                  //Idle While Empty
                
               // LEDPORT |= ALMLED;       //Debug
                if(levelSensor[0].sensorRead == 0 && counter1 == 0){     //Full - Rising Edge Start Filter
                    counter1 = 1;
                    levelSensor[0].levelState = TRANSITION_TO_FULL;

                }else{
                    counter1 = 0;
                    //TODO: Sleep
                }
                break;

            case TRANSITION_TO_FULL:                             //Transition to Full

                if(levelSensor[0].sensorRead > 0){                //Temp Full - Back to Idle
                    counter1 = 0;
                    levelSensor[0].levelState = EMPTY;


                }else if(levelSensor[0].sensorRead == 0 && counter1 > FILTERTMR_ISEMPTY){
                    //LEDPORT |= ALMLED;    //Debug
                    counter1 = 0;
                    levelSensor[0].levelState = FULL;

                }
                break;

            
            case FULL :                          //Alarm Started
                //ALMPORT |= LGTOUT;       //Debug
                levelSensor[0].levelState = ALARM_ON;    //Get out of this state
                blinkState = 4;
                alarmState ++;      //Increment Priority Level for multiple sensors
                //function call for enum value - Alarm = nextState;
                break;
        }
//
// /****************Level One State System*************************************/
//    if(lvlOneState == INITIAL_STATE){                           //Initial State - Wait for tub to be empty
//            if(lvlOneRead > 0){             //Tub Empty - Rising Edge
//                lvlOneState = TRANSITION_TO_EMPTY;   //Start Filter to go Down
//            }else {                         //Not Empty Yet - Stop Timer and Counter
//                counter1 = 0;
//            }
//
//
//        }else if(lvlOneState == TRANSITION_TO_EMPTY){                    //Filter Transistion to Empty - Flashing SNSLED
//            //LEDPORT |= ALMLED;      //Debug
//            if(lvlOneRead == 0){      //Full Tub - Restart Filter
//
//                counter1 = 0;
//                lvlOneState = INITIAL_STATE;
//
//            }else if(lvlOneRead > 0 && counter1 >= FILTERTMR_ISFULL){  //Empty Tub - Stop Timer, Next State
//                counter1 = 0;
//                lvlOneState = EMPTY;
//
//            }
//
//        }else if(lvlOneState == EMPTY){                                    //Idle While Empty
//           // LEDPORT |= ALMLED;       //Debug
//            if(lvlOneRead == 0 && counter1 == 0){     //Full - Rising Edge Start Filter
//                counter1 = 1;
//                lvlOneState = TRANSITION_TO_FULL;
//
//            }else{
//                counter1 = 0;
//                //TODO: Sleep
//            }
//
//        }else if(lvlOneState == TRANSITION_TO_FULL){                             //Transition to Full
//
//            if(lvlOneRead > 0){                //Temp Full - Back to Idle
//                counter1 = 0;
//                lvlOneState = EMPTY;
//
//
//            }else if(lvlOneRead == 0 && counter1 > FILTERTMR_ISEMPTY){
//                //LEDPORT |= ALMLED;    //Debug
//                counter1 = 0;
//                lvlOneState = FULL;
//
//            }
//
//
//        }else if(lvlOneState == FULL){                             //Alarm Started
//            //ALMPORT |= LGTOUT;       //Debug
//            lvlOneState = ALARM_ON;    //Get out of this state
//            blinkState = 4;
//            alarmState ++;      //Increment Priority Level for multiple sensors
//            //function call for enum value - Alarm = nextState;
//        }

/***************************Level Two State System****************************/
        switch (levelSensor[1].levelState)
        {

            case INITIAL_STATE :                        //Initial State - Wait for tub to be empty
                if(levelSensor[1].levelState > 0)             //Tub Empty - Rising Edge
                {
                    levelSensor[1].levelState = TRANSITION_TO_EMPTY;   //Start Filter to go Down
                }else                      //Not Empty Yet - Stop Timer and Counter
                {
                    counter2 = 0;
                }


            case TRANSITION_TO_EMPTY :                    //Filter Transistion to Empty - Flashing SNSLED
           
            if(levelSensor[1].levelState == 0)   //Full Tub - Restart Filter
            {
                //LEDPORT |= ALMLED;  //Debug
                counter2 = 0;
                levelSensor[1].levelState = INITIAL_STATE;

            }else if(levelSensor[1].levelState > 0 && counter2 >= FILTERTMR_ISFULL)  //Empty Tub - Stop Timer, Next State
            {
                counter2 = 0;
                levelSensor[1].levelState = EMPTY;
               
            }

            case  EMPTY :                                   //Idle While Empty
               // LEDPORT |= ALMLED;       //Debug
                if(levelSensor[1].levelState == 0 && counter2 == 0)
                {
                    counter2 = 1;
                    levelSensor[1].levelState = TRANSITION_TO_FULL;
                }else
                {
                    counter2 = 0;
                    //TODO: Sleep
                }

            case TRANSITION_TO_FULL :                           //Transition to Full

                if(levelSensor[1].levelState > 0)                //Temp Full - Back to Idle
                {
                    counter2 = 0;
                    levelSensor[1].levelState = EMPTY;
                }else if(levelSensor[1].levelState == 0 && counter2 > FILTERTMR_ISEMPTY)
                {
                    //LEDPORT |= ALMLED;    //Debug
                    counter2 = 0;
                    levelSensor[1].levelState = FULL;
                }

            case FULL :                            //Alarm Started
                //ALMPORT |= LGTOUT;       //Debug
                levelSensor[1].levelState = ALARM_ON;    //Get out of this state
                blinkState = 4;
                alarmState ++;      //Increment Priority Level for multiple sensors
        

        }
/********************ALARM ON***************************************************/
        if(alarmState >= 1)                                   //Alarm On - Flash ALMLED, SNSLED ON
        {

            if(almCounter == 0)                                //Timer Started - LEDS doing their thing
            {
                ALMPORT |= LGTOUT;
                adc_start();
                timer1_start();
                almCounter = 1;

            }else if(almCounter > alarmTimer)                   //Alarm Timer Reached - Engage Siren
            {
                ALMPORT |= ALMOUT |LGTOUT;      
                timer1_stop();
                almCounter = 1;   //Don't allow timer1 to restart
                alarmState = 3; //All Out Siren and Light
            }
        }


        blinkLed(levelSensor[0].levelState, levelSensor[1].levelState, alarmState, &blinkState);

    }


    //NOT A GOOD SIGNAL!! ERROR!

    while(1)
    {
        LEDPORT^=POWLED;
        for(int i=0;i<10000;i++);

    }
    return (EXIT_SUCCESS);
}

