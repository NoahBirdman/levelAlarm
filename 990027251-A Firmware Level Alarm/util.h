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


    //The states that are allowed
    typedef enum
    {
        INITIAL_STATE,
        TRANSITION_TO_EMPTY,
        EMPTY,
        TRANSITION_TO_FULL,
        FULL,
        ALARM_ON
    }levelStates_t;

    //The options for blinking
    typedef enum
    {
        LIGHTS_OFF,
        FILTER_BLINK_FAST,
        ALARM_BLINK_SLOW,
        ALARM_SOLID_ON

    }blinkStates_t;

    typedef struct{
        levelStates_t levelState;
        uint8_t sensorRead;
        volatile int counter;
    }levelSensor_t;

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

    //Select Appropriate Blinking
    void blinkLed(uint8_t stateOne, uint8_t stateTwo, uint8_t almState, uint8_t *blinkState);

#ifdef	__cplusplus
}
#endif

#endif	/* UTIL_H */

