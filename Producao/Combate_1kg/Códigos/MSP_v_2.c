#include <msp430g2253.h>


/***************************************************************************
 * T�tulo: C�digo de Controle de movimenta��o para rob�s
 * de combate.
 *
 * Descri��o: Consiste em uma op��o de algor�tmo que pode ser implementado
 * no controle de movimenta��o, em coordenadas cartesianas, de rob�s que
 * possuem pelomenos 2 motores em cada lado. A movimenta��o e l�gica utili-
 * zada baseia-se no controle de apenas  um Joystick.
 *
 * Autores:
 *
 *- Daniel Carvalho.
 *- Guilherme Felix.
 *
 * Data: 22/08/2017
 * Modificado em: 31/09/2017
 *
 * - Eletr�nica de Combate - Equipe de competi��es TITANS� - UnB-Gama
 * **************************************************************************
 */


#define left_forward BIT0 // inputs para a ponte h
#define left_reverse BIT3
#define right_forward BIT4
#define right_reverse BIT5

#define R TA1CCR1    // PWM do canal 1 (enables para ponte h)
#define L TA1CCR2    // PWM do canal 2

// Vari�veis globais
//unsigned int T0 = 0;
unsigned int Pulse_1 = 0, Pulse_2 = 0;
int coord_X = 0,coord_Y = 0;


// Fun��es
void Fail_safe(void){
    Pulse_1 = 1000;
    Pulse_2 = 1000;
}
long map(long Accel, long in_min, long in_max, long out_min, long out_max)
{
  return (Accel - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Pulse_read(void){
    TA0CCTL0 = CAP + SCS + CCIS_0 + CM_3;
    while ((TA0CCTL0 & CCIFG) == 0);
    if ((P1IN & BIT1) != 0){
        //T0 = 0;
        TA0CCTL0 &= ~CCIFG;
        TA0R = 0;
        TA0CTL = TASSEL_2 + MC_2;
        while((TA0CCTL0 & CCIFG) == 0);
        Pulse_1 = TA0CCR0;
        TA0R = 0;
        TA0CCTL1 = CAP + SCS + CCIS_0 + CM_3;
        while ((TA0CCTL1 & CCIFG) == 0);
        Pulse_2 = TA0CCR1;
    }
    //if(Pulse_1 < 900 || Pulse_2 < 900 || Pulse_1 > 2100 || Pulse_2 > 2100)
    //    Fail_safe();
    //return T0;
}
// Configura��es principais encontram-se na main
int main(void) {

    // Watchdog timer e clock Set-Up
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;
    P1DIR = 0xFF;

    // PWM output Set-Up
    P2DIR |= BIT1 | BIT4; // P2.1 e P2.4 como saida
    P2SEL |= BIT1 | BIT4;

    // Timer1 Set-Up (PWM)
    TA1CCR0 = 1000-1;       // Periodo do PWM (F = 1kHz)
    TA1CCTL1 |= OUTMOD_7;   // Registrador como modo reset/set (Timer 1.1)
    TA1CCTL2 |= OUTMOD_7;   // Registrador como modo reset/set (Timer 1.2)
    TA1CTL |= TASSEL_2 + MC_1;
    TA1CCR1 = 0; // PWM duty cycle 1 (deve mudar via software).
    TA1CCR2 = 0; // PWM duty cycle 2 - CCR1/(CCR0+1) * 100

    // Capture input Set-Up
    P1DIR &= ~(BIT1 + BIT2);
    P1SEL |= BIT1 | BIT2;
    P1REN &= ~(BIT1 + BIT2);
    P1OUT |= (BIT1 + BIT2);
    //P2SEL &= ~ (BIT1 + BIT2);

    // Timer0 Set-Up (Capture)
    TA0CTL = TASSEL_2 + MC_3;   // Modo cont�nuo de contagem e sync com SMclock.
    // captura up/down + sync com SM clock + interrupt enable P1.1.
    //TA0CCTL1 = CAP + SCS + CCIS_0 + CM_3; // captura up/down + sync com SM clock + interrupt enable P1.2.
    //__bis_SR_register(CPUOFF + GIE); // Habilitando interrup��o e aguardando a leitura dos sinais.

    for(;;){

        Pulse_read();

        coord_X = map((long)Pulse_1,1000,2000,-255,255);
        coord_Y = map((long)Pulse_2,1000,2000,-255,255);

        if(abs(coord_X) < 20 && abs(coord_Y) < 20){ // --> Nessa configura��o, valores entre os intervalos(-20 -> 20) ser�o considerados como Zero.
          L = 0;                                    // Isso foi feito para desligar os dois motores quando a alavanda estiver na "posi��o 0",
          R = 0;                                    // visto que o sinal sofre varia��o de at� -8 a 8 na posi��o inicial do "joystick".
          P1OUT &= ~(left_reverse + left_forward + right_forward + right_reverse);

        }else if(abs(coord_X) >= 20 || abs(coord_Y) >= 20){
          if(coord_X < 0 && coord_Y < 0 && coord_X >= coord_Y ){ // 3� quadrante(6�regi�o no mapa).
            L = abs(coord_Y - coord_X);
            R = abs(coord_Y);
            P1OUT |= (right_reverse + left_reverse);
            P1OUT &= ~(right_forward + left_forward);
          }else if(coord_X < 0 && coord_Y < 0 && coord_X < coord_Y){ // 3� quadrante(5� regi�o do mapa).
            R = abs((-coord_X) + coord_Y);
            L = abs(coord_X - coord_Y);
            P1OUT |=(left_reverse + right_forward);
            P1OUT &= ~(left_forward + right_reverse);
          }else if(coord_X < 0 && coord_Y > 0 && (coord_Y >= - coord_X)){ // 2� quadrante (3� regi�o do mapa).
            R = coord_Y;
            L = (coord_Y + coord_X);
            P1OUT |= (right_forward + left_forward);
            P1OUT &= ~(right_reverse + left_reverse);
          }else if(coord_X < 0 && coord_Y > 0 && (coord_Y < - coord_X)){ // 2� quadrante (4� regi�o do mapa).
            R = (- coord_X);
            L = abs(coord_Y + coord_X);
            P1OUT |= (left_reverse + right_forward);
            P1OUT &= ~(left_forward + right_reverse);
           }else if(coord_X > 0 && coord_Y < 0 && ( - coord_Y >= coord_X)){ // 4� quadrante (7� regi�o do mapa).
           R = abs(coord_Y  + coord_X);
           L = abs(coord_Y);
           P1OUT |= (right_reverse + left_reverse);
           P1OUT &= ~(right_forward + left_forward);
          }else if(coord_X > 0 && coord_Y < 0 && ( - coord_Y < coord_X)){ // 4� quadrante (8� regi�o do mapa).
            R = abs((-coord_X) - coord_Y);
            L = (coord_X + coord_Y);
            P1OUT |= (left_forward + right_reverse);
            P1OUT &= ~(left_reverse + right_forward);
          }else if(coord_X > 0 && coord_Y > 0 && coord_Y  >= coord_X){ // 1� quadrante (2� regi�o do mapa).
            R = (coord_Y - coord_X);
            L = coord_Y;
            P1OUT |= (right_forward + left_forward);
            P1OUT &= ~(right_reverse + left_reverse);
          }else if(coord_X > 0 && coord_Y > 0 && (coord_Y < coord_X)){ // 1� quadrante (1� regi�o do mapa).
            R = abs(coord_Y - coord_X);
            L = coord_X;
            P1OUT |= (left_forward + right_reverse);
            P1OUT &= ~(left_reverse + right_forward);
          }
        }

    }
}
// **********************************************************************
// A ideia � deixar a CPU desligada para fazer a leitura dos 2 sinais.
// A interrup��o vai ser chamada 2 vezes para cada leitura, visto que
// precisamos determinar o inicio e fim do pulso. Ao fim da leitura, a
// interrup��o "liga" a CPU denovo para o processamento.
//
// OBS: Devemos garantir que o processamento seja mais r�pido que a fre-
// quencia do sinal. A interrup��o n�o deve "atrapalhar" o processamento.
// ***********************************************************************
/*
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void){
    if((TA0CCTL0 & COV) ||(TA0CCTL1 & COV)){
        Pulse_1 = 0;
        Pulse_2 = 0;
        TA0CCTL0 &= ~COV;
        TA0CCTL1 &= ~COV;
    }

    if(enable == 0){
        TA0CTL = TASSEL_2 + MC_2; // Inicio da contagem.
        enable ++;
    }
    else if(enable == 1){
        enable ++;
        //if((P1IN & BIT2) != 0){   // Mesmo procedimento para o Canal 2.
        Pulse_1 = TA0CCR0;
        TA0R = 0;
    }else{
            enable = 0;
            Pulse_2 = TA0CCR1;
            TA0CTL &= ~MC_3;
            TA0R = 0;
            __bic_SR_register_on_exit(CPUOFF);  // "Acorda" a CPU
    }
    TA0CCTL1 &= ~CCIFG;
    TA0CCTL0 &= ~CCIFG;

}*/
