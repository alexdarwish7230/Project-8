#include <xc.h>
#include <p18f4620.h>
#include "Interrupt.h"


unsigned char bit_count;                                                        // constant declarations
unsigned int Time_Elapsed;

unsigned long long Nec_code;
unsigned char Nec_State = 0;

extern char Nec_Button;
extern short Nec_OK;

void Init_Interrupt()
{
    INTCON3bits.INT2IF = 0;                                                     // Clear external interrupt INT2IF
    INTCON2bits.INTEDG2 = 0;                                                    // Edge programming for INT 2 falling edge H to L
    INTCON3bits.INT2IE = 1;                                                     // Enable external interrupt INT2IE

    TMR1H = 0;                                                                  // Reset Timer1
    TMR1L = 0;                                                                  //
    PIR1bits.TMR1IF = 0;                                                        // Clear timer 1 interrupt flag
    PIE1bits.TMR1IE = 1;                                                        // Enable Timer 1 interrupt
    INTCONbits.PEIE = 1;                                                        // Enable Peripheral interrupt
    INTCONbits.GIE = 1;                                                         // Enable global interrupts
}

void Enable_INT_Interrupt()
{
    INTCON3bits.INT2IE = 1;                                                     // Enable external interrupt
    INTCON2bits.INTEDG2 = 0;                                                    // Edge programming for INT2 falling edge
}    

void interrupt high_priority chkisr()
{
    if (PIR1bits.TMR1IF == 1) TIMER1_ISR();
    if (INTCON3bits.INT2IF == 1) INTx_ISR();
}

void TIMER1_ISR(void)
{
    Nec_State = 0;                                                              // Reset decoding process
    INTCON2bits.INTEDG2 = 0;                                                    // Edge programming for INT falling edge
    T1CONbits.TMR1ON = 0;                                                       // Disable T1 Timer
    PIR1bits.TMR1IF = 0;                                                        // Clear interrupt flag
}

void Reset_Nec_State()                                                          
{
    Nec_State=0;                                                                // set Nec_State to 0
    T1CONbits.TMR1ON = 0;                                                       // Disable T1 timer
}

void INTx_ISR(void)
{
    INTCON3bits.INT2IF = 0;                                                     // Clear external interrupt INT2IF
    if (Nec_State != 0)
    {
        Time_Elapsed = (TMR1H << 8) | TMR1L;                                    // Store Timer1 value
        TMR1H = 0;                                                              // Reset Timer1
        TMR1L = 0;
    }
    switch(Nec_State)
    {
        case 0 :
        {
                                                                                // Clear Timer 1
            TMR1H = 0;                                                          // Reset Timer1
            TMR1L = 0;                                                          //
            PIR1bits.TMR1IF = 0;                                                // Clear flag
            T1CON= 0x90;                                                        // Program Timer1 mode with count = 1usec using System clock running at 8Mhz
            T1CONbits.TMR1ON = 1;                                               // Enable Timer 1
            bit_count = 0;                                                      // Force bit count (bit_count) to 0
            Nec_code = 0;                                                       // Set Nec_code = 0
            Nec_State = 1;                                                      // Set Nec_State to state 1
            INTCON2bits.INTEDG2 = 1;                                            // Change Edge interrupt of INT 2 to Low to High            
            return;
        }
        case 1 :
        {
            if((Time_Elapsed >= 8500) && (Time_Elapsed <= 9500))                // if value is between 8500 usec and 9500 usec
                Nec_State = 2;                                                  // set Nec_State to 2
            else{Reset_Nec_State();}                                            // if not then reset 
            
            INTCON2bits.INTEDG2 = 0;                                            // change edge interrupt of INT2 to high to low
            return;   
        }
        
        case 2 :                            
        {
            if((Time_Elapsed >= 4000) && (Time_Elapsed <= 5000))                // if value is between 4000 usec and 5000 usec
                Nec_State = 3;                                                  // set Nec_State to 3
            else{Reset_Nec_State();}                                            // if not then reset 
            
            INTCON2bits.INTEDG2 = 1;                                            // change edge interrupt of INT2 to low to high
            return;                                                             
        }
        
        case 3 :                            
        {
            if((Time_Elapsed >= 400) && (Time_Elapsed <= 700))                  // if value is between 4000 usec and 5000 usec
                Nec_State = 4;                                                  // set Nec_State to 4
            else{Reset_Nec_State();}                                            // if not then reset 
            
            INTCON2bits.INTEDG2 = 0;                                            // change edge interrupt of INT2 to high to low
            return; 
        }
        
        case 4 :                            
        {
            if((Time_Elapsed >= 400) && (Time_Elapsed <= 1800))                 // if value is between 400 usec and 1800usec
            {
                Nec_code = Nec_code << 1;                                       // shift Nec_code by 1
                if(Time_Elapsed >= 1000)                                        // check if greater than 1000
                {                                                               
                    Nec_code++;                                                 // increment Nec_code
                }
                bit_count++;                                                    // increment bit_count++
                if (bit_count > 31)                                             // check if bit_count greater than 31
                {
                    Nec_OK = 1;                                                 // set Nec_OK to 1
                    INTCON3bits.INT2IE = 0;                                     // enable external interrupt
                    Nec_Button = (char)(Nec_code >> 8);                         // shift Nec_code by 8 to the right and save it to Nec_button
                    Nec_State = 0;                                              // set Nec_State to 0
                }
                else                                                            
                    Nec_State = 3;                                              // if bit_count not greater than 31 then set Nec_State to 3
            }        
            INTCON2bits.INTEDG2 = 1;                                            // change edge interrupt of INT2 to low to high
            return;                                                 
        }
    }
}



