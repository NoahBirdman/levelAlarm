/*
 * File:   util.h
 * Author: Noah Bergman
 *
 * Created on April 16, 2015, 11:57 AM
 */

#ifndef UTIL_H
#define	UTIL_H

#ifdef	__cplusplus
extern "C" {
#endif
    #include "alarm_source.h"
    #include <xc.h>
    #include <stdio.h>
    #include <stdint.h>
    

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


    #define ADCPIN (1<<0)   //RA0
    #define ADCTRIS TRISA
    #define ADCPORT PORTA   //ADC used for tuning siren delay

    //Delays for Timers
    const int FILTERTMR_TO_FULL = 50;   //Counter going from Full to Empty
    const int FILTERTMR_TO_EMPTY = 50;   //Counter going from Empty to Full



    //The options for blinking
    typedef enum blinkStates
    {
        LIGHTS_OFF,
        POWER_ON,
        FILTER_BLINK_FAST,
        ALARM_BLINK_SLOW,
        ALARM_SOLID_ON

    }blinkStates_t;


    //The states that are allowed
    typedef enum levelStates
    {
        INITIAL_STATE,
        TRANSITION_TO_EMPTY,
        EMPTY,
        TRANSITION_TO_FULL,
        TURN_ON_ALARM,
        FINAL_STATE
    }levelStates_t;

    typedef enum tankStates
    {
        TANK_IS_FULL,
        TANK_IS_EMPTY
    }tankStates_t;

    typedef struct levelSensor{
        enum levelStates  LEVEL_STATE;
        uint8_t sensorRead;
        enum tankStates TANK_STATE;
        volatile int counter;
    }levelSensor_t;



    //Take the pin input and convert to full or empty tank
    void checkTankStatus(levelSensor_t *theTankSensor);

    //Check the states and inputs for a sensor
    void checkSensorState(struct levelSensor *theSensor);

    //Initiallizes the variables to zero
    void init_sensor(levelSensor_t *theSensor_init);
    
    //Select Appropriate Blinking
    void blinkLed(enum levelStates *stateOne, enum levelStates *stateTwo, alarmStates_t *almState, blinkStates_t *blinkState);

    //Timer 1 Initialize
    void timer1_init(void);

    //Timer1 Start
    void timer1_start(void);

    //Timer 1 Stop
    void timer1_stop(void);

    //Timer 2 Initialize
    void timer2_init(void);

    //Timer 2 Start
    void timer2_start(void);

    //Timer 2 Stop
    void timer2_stop(void);

    //Timer 0 Initialize
    void timer0_init(void);

    //Timer 0 Start
    void timer0_start(void);

    //Timer 0 Stop
    void timer0_stop(void);

    //ADC Initialize
    void adc_init_CH0(void);

    //Start ADC
    void adc_start(void);


    

#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

