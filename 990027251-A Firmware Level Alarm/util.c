#include "util.h"


void checkTankStatus(levelSensor_t *theSensor){

   // theSensor.sensorRead = SNSPORT & (LVLONE | LVLTWO);
    //Choose whatport??? Save pin and port?
    if (theSensor->sensorRead > 0)
    {
        theSensor->TANK_STATE = TANK_IS_EMPTY;
        //LEDPORT |= ALMLED; //Debug
    }else
    {
        theSensor->TANK_STATE = TANK_IS_FULL;
        //LEDPORT &= ~(ALMLED);     //Debug
    }
    

}
void checkSensorState(struct levelSensor *theSensor)
{
    switch(theSensor->LEVEL_STATE){
            case INITIAL_STATE :                           //Initial State - Wait for tub to be empty
                //LEDPORT &= ~(ALMLED);     //Debug
                //LEDPORT |= ALMLED;      //Debug
                if(theSensor->TANK_STATE == TANK_IS_EMPTY)
                {
                    theSensor->LEVEL_STATE = TRANSITION_TO_EMPTY;
                }else
                {
                    theSensor->counter = 0;
                }
                break;

            case TRANSITION_TO_EMPTY :                   //Filter Transistion to Empty - Flashing SNSLED
                 //LEDPORT &= ~(ALMLED);     //Debug
               // LEDPORT |= ALMLED;      //Debug

                if(theSensor->TANK_STATE == TANK_IS_FULL)      //Full Tub - Restart Filter
                {
                    theSensor->counter = 0;
                    theSensor->LEVEL_STATE = INITIAL_STATE;

                }else if(theSensor->TANK_STATE == TANK_IS_EMPTY && theSensor->counter >= FILTERTMR_TO_EMPTY)//Empty Tub - Stop Timer, Next State
                {
                    theSensor->counter = 0;
                    theSensor->LEVEL_STATE = EMPTY;

                }
                break;

            case EMPTY:                                  //Idle While Empty
                //LEDPORT |= ALMLED;       //Debug
                if(theSensor->TANK_STATE == TANK_IS_FULL && theSensor->counter == 0){     //Full - Rising Edge Start Filter
                    theSensor->LEVEL_STATE = TRANSITION_TO_FULL;

                }else{
                    theSensor->counter = 0;
                    //TODO: Sleep
                }
                break;

            case TRANSITION_TO_FULL:                             //Transition to Full

                if(theSensor->TANK_STATE == TANK_IS_EMPTY){                //Temp Full - Back to Idle
                    theSensor->counter = 0;
                    theSensor->LEVEL_STATE = EMPTY;


                }else if(theSensor->TANK_STATE == TANK_IS_FULL && theSensor->counter > FILTERTMR_TO_FULL){
                    //LEDPORT |= ALMLED;    //Debug
                    theSensor->counter = 0;
                    theSensor->LEVEL_STATE = TURN_ON_ALARM;

                }
                break;


            case TURN_ON_ALARM :                          //Alarm Started
                //ALMPORT |= LGTOUT;       //Debug
                //Do Nothing at this point
                break;
        }
}



//Initialize all variables to
void init_sensor(levelSensor_t *theSensor_init)
{
    theSensor_init->counter = 0;
    theSensor_init->LEVEL_STATE = INITIAL_STATE;
    theSensor_init->sensorRead = 0;
    theSensor_init->TANK_STATE = TANK_IS_EMPTY;

}

void blinkLed(enum levelStates *stateOne, enum levelStates *stateTwo, alarmStates_t *almState, enum blinkStates *blinkState)
{
    if(*stateOne == TRANSITION_TO_EMPTY || *stateOne == TRANSITION_TO_FULL || *stateTwo == TRANSITION_TO_EMPTY || *stateTwo == TRANSITION_TO_FULL)
    {
        *blinkState = FILTER_BLINK_FAST ;    //Blink Fast

    }else if(*almState == ALARM_ON || *almState == ALARM_DOUBLE_TIME)
    {
        *blinkState = ALARM_BLINK_SLOW;

    }else if (*almState == ALARM_FINAL_STATE)
    {
        *blinkState = ALARM_SOLID_ON;

    }else
    {
        *blinkState = LIGHTS_OFF;

    }

}

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


