#include "rotary-encoder.h"

void encoderInit(){
    //PORT1
    P1OUT |= (ENCODER_A+ENCODER_B+PUSH_BUTTON); //enable pull-up resistor
    P1REN |= ENCODER_A+ENCODER_B+PUSH_BUTTON;   //enable pull-up resistor
    P1IFG &= ~(ENCODER_A + PUSH_BUTTON);            //clear interupt flag
    P1IE |= ENCODER_A + PUSH_BUTTON;              //enable interupt for encoder

}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{

    if(P1IFG&ENCODER_A)
    {
        if(P1IN & ENCODER_B)
        {
            stepCCW(); //call function for step CCW
        }
        else
        {
            stepCW(); //call function for step CW
        }

        P1IFG &= ~ENCODER_A;    //clear interupt flag
    }

    if(P1IFG&PUSH_BUTTON)
    {

        stepPush();
        __delay_cycles(400);
        P1IFG &= ~PUSH_BUTTON;    //clear interupt flag
    }
}

