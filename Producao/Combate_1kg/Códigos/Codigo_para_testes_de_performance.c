#include <msp430.h> 

/*
     This is a code designed to help engineers to
 calculate the time execution of a function,  base-
 ed on the Capture event of timerA.All the contents
 below are presented on the datasheet of MSP340G2553
 and in the user guide provided by Texas Instruments.

 Author: Daniel Carvalho de Sousa
 Date: 20/08/2017

 */
static unsigned int st[8];                      // Servo transmit times
static unsigned int adc_channels[6];
long int x0,y0;
int y = 100;
int x = 500;
unsigned int T0 = 0,enable = 0;
float Temp_exec = 0;
/*
long map(long Accel, long in_min, long in_max, long out_min, long out_max)
{
  return (Accel - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}*/
void Setup_ADC(void)
{
      ADC10CTL1 = INCH_5 + CONSEQ_1;            // input channel A5 -> A0. (CONSEQ_1) sequence of channels.
      ADC10CTL0 = ADC10SHT_2 + MSC + ADC10ON;   // 8x ADC clocks hold time (ADC10SHT_2). Multiple Sample conversion(MSC). Enable adc(ADC10ON). Habilita interrupt do adc(ADC10IE).
      ADC10DTC1 = 6;                            // Number of conversions
      ADC10AE0 |= 0x3F;                         // Disable digital I/O pins P1.5 - P1.0 (Analog Enable).
}
void Read_Signal(void)
{
    __bic_SR_register(GIE);
    ADC10CTL0 &= ~ENC;
    while (ADC10CTL1 & BUSY);       // Wait if ADC10 core is active
    ADC10SA = (unsigned int)adc_channels;     // Copies data in ADC10SA to unsigned int adc array.The DTC transfers the word-value of ADC10MEM to the address pointer ADC10SA.
    ADC10CTL0 |= ENC + ADC10SC;     // Sampling and conversion start, one block data transfer (ADC10TB).
    while((ADC10CTL0 & ADC10IFG) == 0);    // Wait until conversion finished.
    __bis_SR_register(GIE);
}
/*
static void init_ppm_tx(unsigned pol)           // - Initialize PPM transmission
{                                               // pol = 0: idle low, pulse high
                                                // pol = 1: idle high, pulse low

    P2DIR |= BIT0;                              // PPM output on P2.0
    P2SEL |= BIT0;                              // Enable timer compare output
    P2SEL2 &= ~BIT0;

    TA1CCTL0 = OUTMOD_0 | (pol ? OUT : 0);      // Set initial state of output (polarity)
    TA1CCTL0 = OUTMOD_4 | CCIE;                 // Set timer output to toggle mode, enable interrupt
    TA1CCR0 = TA1R + 10000;                      // Set initial interrupt time
}
*/


//static const unsigned pulse_duration = 200;     // Duration of on time of each pulse
//static const unsigned frame_duration = 20000;   // Total duration of a complete frame


int main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop watchdog timer
    BCSCTL1 = CALBC1_1MHZ;  // select 1Mhz clock.
    DCOCTL = CALDCO_1MHZ;   // select the DCO clock.
	TA0CCTL0 = CAP + SCS + CCIS1 + CM_3 + CCIE; // both edges capture + sync with the clock + interrupt flag.
    __bis_SR_register(GIE); // set the global interruptions
	TA0CTL = TASSEL_2 + MC_2; // timer´s clock is system master clock, whithout division.
	TA0CCTL0 ^= CCIS0;  // Interrupt request (capture condition)
	TA0CTL = TASSEL_2 + MC_2;   // Reconfigure timer
    P2REN = BIT1 | BIT2 | BIT3 | BIT4;   // pulldown resistors for digital inputs
    P2OUT &= ~ (BIT1 | BIT2 | BIT3 | BIT4);
    Setup_ADC();
    int i = 0;
	//---- Insert the code test here ! -----

    Read_Signal();

    for(i = 5; i >= 0; i--){ // Reordering vector due to match with PCB
        //st[i + 1] = st[i - 1] << 1;
        st[5 - i] = adc_channels[i] << 1;
    }
    st[0] += 620;  // simple adjustment to fit between 1000 - 2000
    st[1] += 530;

    if(P2IN & BIT1){     // Channels 7 and 8 are pushbuttons.
        st[6] = 2000;
    }else{
        if(P2IN & BIT2)
            st[6] = 1500;
        else
            st[6] = 1000;
    }
    if(P2IN & BIT3){
        st[7] = 2000;
    }else{
        if(P2IN & BIT4)
            st[7] = 1500;
        else
            st[7] = 1000;
    }
	//--------------------------------------
	TA0CCTL0 ^= CCIS0;  // Last interrupt request (final capture condition)
	Temp_exec *= 0.001;

	__bis_SR_register(CPUOFF);
	return 0;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void){
    TA0CTL &= ~MC_3;    // Stop the timer (holds its value until the start)
    if(enable == 0){
        enable ++;
        T0 = TA0CCR0 + 9; //reason: calling an interrupt and "stop timer command" demand 9 clock cycles.
    }else{
        enable = 0;
        Temp_exec = TA0CCR0 - T0 - 4; // 4 clocks demanded of the interrupt function.
    }
    TA0CCTL0 &= ~CCIFG;
}
