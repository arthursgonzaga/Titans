#include <msp430g2253.h>

int main(void) {

    // Watchdog timer e clock Set-Up
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

    // Input/Output Set-Up
    P1DIR |= BIT2;                  // P1.2 output
    P1SEL |= BIT2;                  // selecionar P1.2 Timer0_A Output
    P2DIR |= BIT1;                  // P2.1 output
    P2SEL |= BIT1;                  // selecionar P2.1 Timer1_A Output


    // Timer0_A Set-Up
    TA0CCR0 |= 1000 - 1;             // Periodo do PWM
    TA0CCTL1 |= OUTMOD_7;           // Registrador como saída TA0CCR1 = reset/set
    TA0CCR1 |= 40;                  // PWM duty cycle
    TA0CTL |= TASSEL_2 + MC_1;

    // Timer1_A Set-Up
    TA1CCR0 |= 1000 - 1;             // Periodo PWM
    TA1CCTL1 |= OUTMOD_7;            // Registrador como saída TA1CCR1 = reset/set
    TA1CCR1 |= 4000;                 // PWM duty cycle
    TA1CTL |= TASSEL_2 + MC_1;

    _BIS_SR(LPM0_bits);
}
