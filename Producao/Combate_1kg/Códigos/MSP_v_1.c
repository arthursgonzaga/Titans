#include <msp430g2253.h>


/***************************************************************************
 * Título: Código de Controle de movimentação para robôs
 * de combate.
 *
 * Descrição: Consiste em uma opção de algorítmo que pode ser implementado
 * no controle de movimentação, em coordenadas cartesianas, de robôs que
 * possuem pelomenos 2 motores em cada lado. A movimentação e lógica utili-
 * zada baseia-se no controle de apenas  um Joystick.
 *
 * Autores:
 *
 *- Daniel Carvalho.
 *- Gilvan Camargo.
 *- Guilherme Felix.
 *
 * Data: 22/08/2017
 * Modificado em: 22/08/2017
 *
 * - Eletrônica de Combate - Equipe de competições TITANS® - UnB-Gama
 * **************************************************************************
 */


#define right_reverse BIT0   //Nomeando os enables.
#define right_forward BIT3
#define left_reverse BIT4
#define left_forward BIT5

#define PWM_R BIT6
#define PWM_L BIT7

// Variáveis globais
unsigned int T0 = 0;
unsigned int Pulse_1 = 0, Pulse_2 = 0;
int coord_X = 0,coord_Y = 0,R,L;        //Definindo R e L constantes inteiras

// Funções
void Fail_safe(void){
    TA1CCR1 = 0;
    TA1CCR2 = 0;
}
long map(long Accel, long in_min, long in_max, long out_min, long out_max)
{
  return (Accel - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void Pulse_read(void){
    while ((TA0CCTL0 & CCIFG) == 0){
       //T0 ++;
       if(T0 > 20000) break;
    }
    if ((P1IN & BIT1) != 0){
        T0 = 0;
        TA0CCTL0 &= ~CCIFG;
        TA0R = 0;
        TA0CTL = TASSEL_2 + MC_2;
        while((TA0CCTL0 & CCIFG) == 0){
            //T0 ++;
            if(T0 > 20000) break;
        }
        Pulse_1 = TA0CCR0;
        TA0R = 0;
        TA0CCTL1 = CAP + SCS + CCIS_0 + CM_3;
        while ((TA0CCTL1 & CCIFG) == 0){
            //T0 ++;
            if(T0 > 20000) break;
        }
        Pulse_2 = TA0CCR1;
    }
    //return T0;
}
// Configurações principais encontram-se na main
int main(void) {

    // Watchdog timer e clock Set-Up
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;
    DCOCTL = CALDCO_1MHZ;

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
    //P2SEL &= ~ (BIT1 + BIT2);

    // Timer0 Set-Up (Capture)
    TA0CTL = TASSEL_2 + MC_3;   // Modo contínuo de contagem e sync com SMclock.
    TA0CCTL0 = CAP + SCS + CCIS_0 + CM_3 + CCIE; // captura up/down + sync com SM clock + interrupt enable P1.1.
    TA0CCTL1 = CAP + SCS + CCIS_0 + CM_3 + CCIE; // captura up/down + sync com SM clock + interrupt enable P1.2.
    //__bis_SR_register(CPUOFF + GIE); // Habilitando interrupção e aguardando a leitura dos sinais.

    for(;;){
        Pulse_read();
        //if (enable != 0){
            coord_X = map(Pulse_1,1000,2000,-255,255);
            coord_Y = map(Pulse_2,1000,2000,-255,255);
		
		//------------Controle de coordenadas--------------

if(coord_X >= -20 && coord_X <= 20 && coord_Y >= -20 && coord_Y <= 20){ // --> Nessa configuração, valores entre os intervalos(-10,10) serão considerados como Zero.
  L = 0;                                                                // Isso foi feito para desligar os dois motores quando a alavanda estiver na "posição 0",
  R = 0;                                                                // visto que o sinal sofre variação de até -8 a 8 na posição inicial do "joystick".                                          //   bô será giratório, de 0 a 360°. Isso será feito com os motores ligados em sentidos opostos.
}else if(coord_X > 20 || coord_X < -20 || coord_Y > 20 || coord_Y < -20){
  if(coord_X < 0 && coord_Y < 0 && (coord_X >= coord_Y)){
    R = coord_Y;
    L = coord_Y - coord_X;
  }else if(coord_X < 0 && coord_Y > 0 && (coord_Y >= - coord_X)){
    R = coord_Y;
    L = coord_Y + coord_X;
  }else if(coord_X < 0 && coord_Y > 0 && (coord_Y < - coord_X)){
    R = - coord_X;
    L = coord_Y + coord_X;
  }else if(coord_X > 0 && coord_Y < 0 && ( - coord_Y >= coord_X)){
    R = coord_Y  + coord_X;
    L = coord_Y;
  }else if(coord_X > 0 && coord_Y > 0 && (coord_Y >= coord_X)){
    R = coord_Y - coord_X;
    L = coord_Y; 
  }else if(coord_X > 0 && coord_Y > 0 && (coord_Y < coord_X)){
    R = coord_Y - coord_X;
    L = coord_X; 
  }    
}else{
  R = 0;
  L = 0;
}
//Falta traduzir....
//------------------------------------
// Descomente para testar os valores enviados às portas. 
//Serial.print("MOTOR_L:");
//Serial.print("    ");
//Serial.println("MOTOR_R:");
//Serial.print(L);
//Serial.print("         ");
//Serial.println(R);
				
		if(R < 0){
			Bit 0 = 1;
			BIT 3 = 0;
			BIT 6 =(R*-1);  // Não tenho certeza sobre esse comando, pelo que entendi é assim
		}
		else if(R > 0){
			BIT 0 = 0;
			BIT 3 = 1; 
			BIT 6 = R;  // Não tenho certeza sobre esse comando, pelo que entendi é assim
		}
		if(L > 0){
		BIT 4= 1;
		BIT 5 = 1;
		BIT 7 = L;  //Não tenho certeza sobre esse comando, pelo que entendi é assim
		}
		if(L < 0){
		BIT 4 = 1;
		BIT 5 = 0;
		bit 7 = L*-1;  // Não tenho certeza sobre esse comando, pelo que entendi é assim
		}
		if(L == 0 || R == 0){
		BIT 4 = 0;
		BIT 5 = 0;  
		BIT 0 = 0;
		BIT 3 = 0;  
		}
		}

        //}else{
        //    Fail_safe();
        //}
    }
}
// **********************************************************************
// A ideia é deixar a CPU desligada para fazer a leitura dos 2 sinais.
// A interrupção vai ser chamada 2 vezes para cada leitura, visto que
// precisamos determinar o inicio e fim do pulso. Ao fim da leitura, a
// interrupção "liga" a CPU denovo para o processamento.
//
// OBS: Devemos garantir que o processamento seja mais rápido que a fre-
// quencia do sinal. A interrupção não deve "atrapalhar" o processamento.
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
