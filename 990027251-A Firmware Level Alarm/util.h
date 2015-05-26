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

