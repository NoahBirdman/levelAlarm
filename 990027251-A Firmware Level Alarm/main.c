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

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#include "alarm_source.h"
#include "util.h"


struct  levelSensor levelSensors[2];
enum blinkStates blinkState = LIGHTS_OFF; //Determines the blinking pattern of ALMLED
volatile struct Alarm theAlarm;

void interrupt ISR(){
    static int adcResult = 0;
    static int blinkCounter =0;
   // TODO: Fix patterns for ALMLED Blinking

   //Timer 0 Overflow - Filter for LVL One
    if(TMR0IF && TMR0IE){

        switch(blinkState)
        {
            case FILTER_BLINK_FAST:
                if((blinkCounter % 5) == 0)
                {
                    LEDPORT ^= ALMLED;
                }
                break;

            case ALARM_BLINK_SLOW:
                if((blinkCounter % 15) == 0)
                {
                    LEDPORT ^= ALMLED;
                }
                break;

            case ALARM_SOLID_ON:
                LEDPORT |= ALMLED;
                break;

            case LIGHTS_OFF:
            default:
                LEDPORT &= ~(ALMLED);
        }
        blinkCounter ++;
        blinkCounter %= 100;
        TMR0IF = 0;
    }
    
    //Timer 1 Overflow
    if(TMR1IE && TMR1IF){


        switch(theAlarm.ALARM_STATE){
            case ALARM_ON:
                theAlarm.current_value += 1;
                break;
            case ALARM_DOUBLE_TIME:
                theAlarm.current_value += 3;
                break;
            case ALARM_FINAL_STATE:
                timer1_stop();
        }

        TMR1IF = 0;
    }

    //Timer 2 Overflow - Filter Counter for both Sensors
    if(TMR2IF && TMR2IE){
        if(levelSensors[0].LEVEL_STATE == TRANSITION_TO_EMPTY || levelSensors[0].LEVEL_STATE == TRANSITION_TO_FULL){   //Transition States
            levelSensors[0].counter++;
        }
        
        if(levelSensors[1].LEVEL_STATE == TRANSITION_TO_EMPTY || levelSensors[1].LEVEL_STATE == TRANSITION_TO_FULL){   //Transition States
            levelSensors[1].counter++;
        }
        TMR2IF = 0;
    }

    //ADC Interrupt Flag
    if(ADIF && ADIE){
       adcResult = (ADRESH <<8) | (ADRESL);
       theAlarm.trigger_value = adcResult >> 2;
       ADIF = 0;
    }
    


}




int main() {

    ADCTRIS |= ADCPIN;
    SNSTRIS |= LVLONE|LVLTWO;
    LEDTRIS &= ~(POWLED|ALMLED);
    ALMTRIS &= ~(LGTOUT|ALMOUT);

    
    init_sensor(&levelSensors[0]);
    init_sensor(&levelSensors[1]);



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

       levelSensors[0].sensorRead = (SNSPORT & LVLONE);
       levelSensors[1].sensorRead = (SNSPORT & LVLTWO);

       //checkTankStatus(&levelSensors[0]);
       checkTankStatus(&levelSensors[1]);

       //checkSensorState(&levelSensors[0]);
       checkSensorState(&levelSensors[1]);
       hello_world();
       
       //checkAlarmState(&levelSensors[1], &theAlarm);
        //checkAlarmState();
       //turnAlarmOn(theAlarm.ALARM_STATE);
        //blinkLed(levelSensors[0].LEVEL_STATE, levelSensors[1].LEVEL_STATE, theAlarm.ALARM_STATE, &blinkState);

    }


    //NOT A GOOD SIGNAL!! ERROR!

    while(1)
    {
        LEDPORT^=POWLED;
        for(int i=0;i<10000;i++);

    }
    return (EXIT_SUCCESS);
}

