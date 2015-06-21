/* 
 * File:   alarm_source.h
 * Author: birdman
 *
 * Created on June 20, 2015, 6:56 PM
 */

#ifndef ALARM_SOURCE_H
#define	ALARM_SOURCE_H

#ifdef	__cplusplus
extern "C" {
#endif

    #include <xc.h>
    #include <stdio.h>
    #include <stdint.h>
    //#include "util.h"


    void hello_world(void);

    typedef enum alarmStates
    {
        ALARM_OFF,
        ALARM_ON,
        ALARM_DOUBLE_TIME,
        ALARM_FINAL_STATE
    }alarmStates_t;

    typedef struct Alarm
    {
        enum alarmStates ALARM_STATE;
        volatile uint8_t current_value;
        volatile uint8_t trigger_value;

    }Alarm_t;

    void checkAlarmState(struct levelSensor *theSensor, struct Alarm *theAlarm);

    void turnAlarmOn(struct Alarm *theAlarm);

    void turnAlarmOff(struct Alarm *theAlarm);


#ifdef	__cplusplus
}
#endif

#endif	/* ALARM_SOURCE_H */

