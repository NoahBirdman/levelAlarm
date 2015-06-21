#include "alarm_source.h"
#include "util.h"

void checkAlarmState(struct levelSensor *theSensor, struct Alarm *theAlarm)
{
    if(theSensor->LEVEL_STATE == TURN_ON_ALARM)
    {
        turnAlarmOn(theAlarm);
    }

    if(theAlarm->current_value > theAlarm->trigger_value)
    {

        theAlarm->ALARM_STATE = ALARM_FINAL_STATE;
        timer1_stop();

    }
    
}

void turnAlarmOn(struct Alarm *theAlarm)
{
    switch(theAlarm->ALARM_STATE)
    {
        case ALARM_OFF:
            adc_start();
            timer1_start();
            theAlarm->current_value = 0;
            theAlarm->ALARM_STATE = ALARM_ON;
            break;
        case ALARM_ON:
        case ALARM_DOUBLE_TIME:
        default:
            theAlarm->ALARM_STATE = ALARM_DOUBLE_TIME;
            break;
    }
    if(theAlarm->ALARM_STATE == ALARM_OFF)
    {
         adc_start();
         timer1_start();
         theAlarm->current_value = 0;

    }

}

void turnAlarmOff(struct Alarm *theAlarm)
{
    timer1_stop();
    theAlarm->ALARM_STATE = ALARM_FINAL_STATE;
    theAlarm->current_value = 0;

}
void hello_world(){


}