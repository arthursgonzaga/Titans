#include <msp430.h>


/***************************************************************************
 * Título: Código de Controle de movimentação para robôs
 * de combate.
 *
 * Descrição: Consiste em uma opção de algorítmo que pode ser implementado
 * no controle de movimentação, em coordenadas cartesianas, de robôs que
 * possuem pelomenos 1 motor em cada lado. A movimentação e lógica utili-
 * zadas baseiam-se no controle de apenas por um Joystick. Com a opção do
 * "Fail-safe", para o caso de perdas de sinal com o rádio controle.
 *
 * Colaboradores:
 *
 *- Daniel Carvalho.
 *- Guilherme Felix.
 *
 * Data de Criação: 22/08/2017
 * Última modificação: 15/10/2017 por Daniel Carvalho.
 *
 * - Eletrônica de Combate - Equipe de competições TITANS® - UnB-Gama
 * **************************************************************************
      A ideia inicial era deixar a CPU desligada para fazer a leitura dos 2
     sinais, porém a interrupção impossibilitou a leitura simultânea dos ca-
     nais, além de dificultar a detecção de falhas.
     A interrupção iria ser chamada 2 vezes para cada leitura, visto que
     precisamos determinar o inicio e fim do pulso. Ao fim da leitura, a
     interrupção "ligaria" a CPU denovo para o processamento.

     OBS: Devemos garantir que o processamento seja mais rápido que a fre-
     quencia do sinal. A interrupção, ou qualquer método de aquisição de da-
     dos não deve "atrapalhar" o processamento no sentido de durar mais que 1
     período do sinal de entrada. Se respeitados esses requisitos, espera-se
     um sistema em Tempo Real com tempo de atualização de 50ms.
 ************************************************************************/
// PWM Left (P2.1)
// PWM Right (P2.4)
// Input channel 1 (P1.1)
// Input channel 2 (P1.2)
// Enables (P1.X):
#define l_f BIT3  // in-4 - inputs para a ponte h
#define l_r BIT0  // in-3
#define r_f BIT5  // in-2
#define r_r BIT4  // in-1

#define L_PWM TA1CCR1    // PWM 1 (duty cycle)
#define R_PWM TA1CCR2    // PWM 2 (duty cycle)

// Variáveis globais
//unsigned int T0 = 0;
unsigned char RX_Data[6];
unsigned char TX_Data[2];
unsigned char RX_ByteCtr;
unsigned char TX_ByteCtr;
unsigned char slaveAddress = 0x68;
unsigned int left_forward  = l_f;
unsigned int right_forward = r_f;
unsigned int left_reverse  = l_r;
unsigned int right_reverse = r_r;
unsigned int L = 0;
unsigned int R = 0;

unsigned int z = 1;
unsigned int Pulse_1 = 0;
unsigned int Pulse_2 = 0; // Variáveis de aquisição de dados
int coord_X = 0,coord_Y = 0; // Valores das coordenadas no algoritmo.
int zAccel;

// Funções
void Fail_safe(void){
    Pulse_1 = 1500;
    Pulse_2 = 1500;
}
long map(long Accel, long in_min, long in_max, long out_min, long out_max)
{
  return (Accel - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/*void Pulse_read(void){
    TA0CCTL0 = CAP + SCS + CCIS_0 + CM_3; // Seta o TA0.0 para operar no modo captura de bordas de subida ou descida.
    while ((TA0CCTL0 & CCIFG) == 0);      // Enquando não ocorrer a captura no canal 1, a função ficará travada nessa linha.
    if ((P1IN & BIT1) != 0){              // Analisa se a captura que ocorreu foi de borda de subida.
        //T0 = 0;
        TA0CCTL0 &= ~CCIFG;               // Limpa a flag de captura.
        TA0R = 0;
        TA0CTL = TASSEL_2 + MC_2;         // Ajusta as configurações do timer e inicia o contador no modo contínuo (até 0xFF).
        while((TA0CCTL0 & CCIFG) == 0);   // espera a segunda captura do canal 1.
        Pulse_1 = TA0CCR0 - 50;                // armazena o valor lido do primeiro canal em "ms" na variável Pulse_1.
        TA0R = 0;                         // Zera o contador.
        TA0CCTL1 = CAP + SCS + CCIS_0 + CM_3;   // Configura o timer TA0.1 para as mesmas configurações do timer TA0.0.
        while ((TA0CCTL1 & CCIFG) == 0);  // Aguarda a captura do canal 2.
        Pulse_2 = TA0CCR1 + 20;                // Armazena o valor do canal 2, em "ms".
    }
    if(Pulse_1 < 900 || Pulse_2 < 900 || Pulse_1 > 2100 || Pulse_2 > 2100)
        Fail_safe();
    //return T0;
}*/
void Pulse_read(void){
    TA0CTL = TASSEL_2 + MC_2;
    TA0CCTL0 = CAP + SCS + CCIS_0 + CM_3; // Seta o TA0.0 para operar no modo captura de bordas de subida ou descida.
    while ((TA0CCTL0 & CCIFG) == 0);      // Enquando não ocorrer a captura no canal 1, a função ficará travada nessa linha.
    if ((P1IN & BIT1) != 0){              // Analisa se a captura que ocorreu foi de borda de subida.
        //T0 = 0;
        TA0CCTL0 &= ~CCIFG;               // Limpa a flag de captura.
        TA0R = 0;
        TA0CCTL1 = CAP + SCS + CCIS_0 + CM_2;
        while(((TA0CCTL0 & CCIFG) == 0)||((TA0CCTL1 & CCIFG) == 0));
        while(((TA0CCTL0 & CCIFG) == 0)||((TA0CCTL1 & CCIFG) == 0));
        Pulse_2 = TA0CCR1 + 15;
        Pulse_1 = TA0CCR0 + 15;
        TA0R = 0;
        //TA0CTL = TASSEL_2 + MC_2;         // Ajusta as configurações do timer e inicia o contador no modo contínuo (até 0xFF).
                                // Zera o contador.
    }
    if(Pulse_1 < 900 || Pulse_2 < 900 || Pulse_1 > 2100 || Pulse_2 > 2100)
        Fail_safe();
}
void i2cInit(void)
{
    // set up I2C module
    UCB0CTL1 |= UCSWRST;                // Enable SW reset
    UCB0CTL0 = UCMST + UCMODE_3 + UCSYNC;           // I2C Master, synchronous mode
    UCB0CTL1 = UCSSEL_2 + UCSWRST;          // Use SMCLK, keep SW reset
    UCB0BR0 = 10;                   // fSCL = SMCLK/12 = ~100kHz
    UCB0BR1 = 0;
    UCB0CTL1 &= ~UCSWRST;               // Clear SW reset, resume operation
}

void i2cWrite(unsigned char address)
{
    //__disable_interrupt();
    IE2 &= ~UCB0TXIE;                   // desabilita o interrupt
    UCB0I2CSA = address;                // Load slave address
    IE2 |= UCB0TXIE;                // Enable TX interrupt
    while(UCB0CTL1 & UCTXSTP);          // Ensure stop condition sent
    UCB0CTL1 |= UCTR + UCTXSTT;         // TX mode and START condition
    //__bis_SR_register(GIE);
    //while((IFG2 & UCB0TXIFG) == 0);
    __bis_SR_register( LPM0_bits + GIE);        // sleep until UCB0TXIFG is set ...
}

void i2cRead(unsigned char address)
{
    //__disable_interrupt();
    IE2 &= ~UCB0RXIE;                   // desabilita o interrupt
    UCB0I2CSA = address;                // Load slave address
    IE2 |= UCB0RXIE;                // Enable RX interrupt
    while(UCB0CTL1 & UCTXSTP);          // Ensure stop condition sent
    UCB0CTL1 &= ~UCTR;              // RX mode
    UCB0CTL1 |= UCTXSTT;                // Start Condition
    //__bis_SR_register(GIE);
    while((IFG2 & UCB0RXIFG) == 0);
    __bis_SR_register(LPM0_bits + GIE);        // sleep until UCB0RXIFG is set ...
}

// Configurações principais encontram-se na main
    int main(void) {

    int count1 = 0;
    int count2 = 0;

    // Watchdog timer e clock Set-Up
    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ;  // A configuração dos timers leva em conta o clock da CPU.(1MHz)
    DCOCTL = CALDCO_1MHZ;   // A mudança nessas definições altera o comportamento do código.
    P1DIR = 0xFF;           // Define todas as portas inutilizadas como saídas.

    // PWM output Set-Up
    P2DIR |= BIT1 | BIT4; // P2.1 e P2.4 como saidas digitais (PWM)
    P2SEL |= BIT1 | BIT4;

    // Timer1 Set-Up (PWM)
    TA1CCR0 = 2000-1;       // Periodo do PWM (F = 500Hz)
    TA1CCTL1 |= OUTMOD_7;   // Registrador como modo reset/set (Timer 1.1)
    TA1CCTL2 |= OUTMOD_7;   // Registrador como modo reset/set (Timer 1.2)
    TA1CTL |= TASSEL_2 + MC_1;
    TA1CCR1 = L; // PWM duty cycle 1 (deve mudar via software).
    TA1CCR2 = R; // PWM duty cycle 2 - CCR1/(CCR0+1) * 100

    // Capture input Set-Up
    P1DIR &= ~(BIT1 + BIT2);    // Define os pinos de captura como entradas.
    P1SEL |= BIT1 | BIT2;       // Condigurando as entradas de captura.
    //P2SEL &= ~ (BIT1 + BI   // Seta os resistores de pullup/down.
    P1OUT |= (BIT1 + BIT2);     // Define os resistores de pullup;
    P1REN &= ~(BIT1 + BIT2);
    // Timer0 Set-Up (Capture)
    //TA0CTL = TASSEL_2 + MC_3;   // Modo contínuo de contagem e sync com SMclock.

    P1SEL |= BIT6 + BIT7;                   // SCL => P1.6
    P1SEL2|= BIT6 + BIT7;                   // SDA => P1.7
    i2cInit();
    slaveAddress = 0x68;                    // MPU-6050 address
    TX_Data[1] = 0x6B;                      // address of PWR_MGMT_1 register
    TX_Data[0] = 0x00;                      // set register to zero (wakes up the MPU-6050)
    TX_ByteCtr = 2;
    i2cWrite(slaveAddress);

    for(;;){

        Pulse_read();

        slaveAddress = 0x68;    // endereço do MPU6050(0x68 ou 0x69)
        TX_Data[0] = 0x3B;      // endereço do reg. de dados do ACELERÔMETRO
        TX_ByteCtr = 1;         // Número de bytes enviados.
        i2cWrite(slaveAddress); // Envia o "request" para o módulo
        // Recebe os dados do ecelerômetro e joga os valores no RX_data
        slaveAddress = 0x68;
        RX_ByteCtr = 6;             // número de bytes a serem recebidos
        i2cRead(slaveAddress);      // Lê os dados enviados pelo MPU6050
        zAccel  = RX_Data[1] << 8;  // MSB
        zAccel |= RX_Data[0];       // LSB

        if(zAccel < 0)    // Nessa condição, inverte os comandos.
            count1 ++;
        else
            count2 ++;
        if((zAccel < 0) && (count1 > 5)){
            left_reverse = r_f;
            left_forward = r_r;
            right_reverse = l_f;
            right_forward = l_r;
            L_PWM = R;
            R_PWM = L;
            count1 = 0;
            count2 = 0;
        }
        if((zAccel > 0) && (count2 > 5)){
            count2 ++;
            left_forward  = l_f;
            right_forward = r_f;
            left_reverse  = l_r;
            right_reverse = r_r;
            R_PWM = R;
            L_PWM = L;
            count1 = 0;
            count2 = 0;
        }

        coord_X = map((long)Pulse_1,800,2200,-1550,1550);
        coord_Y = map((long)Pulse_2,800,2200,-1550,1550);

        if(abs(coord_X) < 50 && abs(coord_Y) < 50){ // --> Nessa configuração, valores entre os intervalos(-50 -> 50) serão considerados como Zero.
          L = 0;                                    // Isso foi feito para desligar os dois motores quando a alavanda estiver na "posição 0",
          R = 0;                                    // visto que o sinal sofre variação de até -8 a 8 na posição inicial do "joystick".
          P1OUT &= ~(left_reverse + left_forward + right_forward + right_reverse);

        }else if((abs(coord_X) >= 50) || (abs(coord_Y) >= 50)){
          if(coord_X < 0 && coord_Y < 0 && coord_X >= coord_Y ){ // 3° quadrante(6°região no mapa).
            L = abs(coord_Y - coord_X);
            R = abs(coord_Y);
            P1OUT |= (right_reverse + left_reverse);
            P1OUT &= ~(right_forward + left_forward);
          }else if(coord_X < 0 && coord_Y < 0 && coord_X < coord_Y){ // 3° quadrante(5° região do mapa).
            R = abs((-coord_X) + coord_Y);
            L = abs(coord_X - coord_Y);
            P1OUT |=(left_reverse + right_forward);
            P1OUT &= ~(left_forward + right_reverse);
          }else if(coord_X < 0 && coord_Y > 0 && (coord_Y >= - coord_X)){ // 2° quadrante (3° região do mapa).
            R = coord_Y;
            L = (coord_Y + coord_X);
            P1OUT |= (right_forward + left_forward);
            P1OUT &= ~(right_reverse + left_reverse);
          }else if(coord_X < 0 && coord_Y > 0 && (coord_Y < - coord_X)){ // 2° quadrante (4° região do mapa).
            R = (- coord_X);
            L = abs(coord_Y + coord_X);
            P1OUT |= (left_reverse + right_forward);
            P1OUT &= ~(left_forward + right_reverse);
           }else if(coord_X > 0 && coord_Y < 0 && ( - coord_Y >= coord_X)){ // 4° quadrante (7° região do mapa).
           R = abs(coord_Y  + coord_X);
           L = abs(coord_Y);
           P1OUT |= (right_reverse + left_reverse);
           P1OUT &= ~(right_forward + left_forward);
          }else if(coord_X > 0 && coord_Y < 0 && ( - coord_Y < coord_X)){ // 4° quadrante (8° região do mapa).
            R = abs((-coord_X) - coord_Y);
            L = (coord_X + coord_Y);
            P1OUT |= (left_forward + right_reverse);
            P1OUT &= ~(left_reverse + right_forward);
          }else if(coord_X > 0 && coord_Y > 0 && coord_Y  >= coord_X){ // 1° quadrante (2° região do mapa).
            R = (coord_Y - coord_X);
            L = coord_Y;
            P1OUT |= (right_forward + left_forward);
            P1OUT &= ~(right_reverse + left_reverse);
          }else if(coord_X > 0 && coord_Y > 0 && (coord_Y < coord_X)){ // 1° quadrante (1° região do mapa).
            R = abs(coord_Y - coord_X);
            L = coord_X;
            P1OUT |= (left_forward + right_reverse);
            P1OUT &= ~(left_reverse + right_forward);
          }
        }

    }
}

#pragma vector = USCIAB0TX_VECTOR
__interrupt void USCIAB0TX_ISR(void)
{

    if(UCB0CTL1 & UCTR)                 // TX mode (UCTR == 1)
    {
        if (TX_ByteCtr)                     // TRUE if more bytes remain
        {
            TX_ByteCtr--;               // Decrement TX byte counter
            UCB0TXBUF = TX_Data[TX_ByteCtr];    // Load TX buffer
        }
        else                        // no more bytes to send
        {
            UCB0CTL1 |= UCTXSTP;            // I2C stop condition
            IFG2 &= ~UCB0TXIFG;         // Clear USCI_B0 TX int flag
            __bic_SR_register_on_exit(CPUOFF);  // Exit LPM0
        }
    }
    else // (UCTR == 0)                 // RX mode
    {
        RX_ByteCtr--;                       // Decrement RX byte counter
        if (RX_ByteCtr)                     // RxByteCtr != 0
        {
            RX_Data[RX_ByteCtr] = UCB0RXBUF;    // Get received byte
            if (RX_ByteCtr == 1)            // Only one byte left?
            UCB0CTL1 |= UCTXSTP;            // Generate I2C stop condition
        }
        else                        // RxByteCtr == 0
        {
            RX_Data[RX_ByteCtr] = UCB0RXBUF;    // Get final received byte
            __bic_SR_register_on_exit(CPUOFF);  // Exit LPM0
        }
    }
}

