
/* 
--------------------------------------------------------------
Código teste para o controle do robô através do controle PWM.
                                                          
- Equipe Titans.

- Autor: Daniel Carvalho.
- Data: 12/02/2017 
--------------------------------------------------------------
 */
const int CH1_input = A0;
const int CH2_input = A1; 
const int CH3_input = A2;
const int CH4_input = A3;
const int left_forward = 13;
const int left_reverse = 12;
const int right_forward = 8;
const int right_reverse = 7;
const int motor_r = 11;
const int motor_l = 10;

//-----------------------------------------------------------------------
// Variáveis responsáveis por mudar a frequência de clock
// do conversor AD, a partir de valores setados nos reg´s.
const unsigned char LEFTY_BITS = (1 << ADLAR); // identa os bits do conversor para a esquerda.
const unsigned char PS_4 = (1 << ADPS1); // preset utilizado (permite 4Mhz de clock);
const unsigned char PS_8 = (1 << ADPS1) | (1 << ADPS0);
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // (preset padrão 16M/128 = 125kHz);
//------------------------------------------------------------------------
int time_1,previous_1;
int time_test0, time_test1;
int time_2,previous_2;
int time_3,previous_3;
int time_4,previous_4;
int X,Y,coord_X,coord_Y, R,L;
void setup() { 
// Escrevendo nos registradores as configurações descritas acima.

  ADCSRA &= ~PS_128;
  ADCSRA |= PS_4;
  ADMUX |= LEFTY_BITS;  
  pinMode(left_forward, OUTPUT);
  pinMode(left_reverse, OUTPUT);
  pinMode(right_forward, OUTPUT);
  pinMode(right_reverse, OUTPUT);
//  Serial.begin(9600);  
}
// ---- Função contagem de tempo ----
int timeup(int channel){
  int counter = 0;                   // counter é iniciado como ZERO e fica assim até
  int sinal = 0;                     // que o nível lógico mudar de LOW para HIGH.
  while(counter == 0){               //  --> Nesse while ele aguarda o nível lógico alto. 
    sinal = analogRead(channel);
    while(sinal > 500){               //  --> Nesse outro ele conta, em proporção ao tempo,
      sinal = analogRead(channel);    //      a partir do momento em que o nível lógico alto está ativo.
      counter = counter + 1;
      delayMicroseconds(1);
    }
  }
  return counter;                     //Retorna o valor da contagem (proporcional ao tempo).
}

void loop(){

time_4 = timeup (CH4_input); 
// Por alguma razão... se lermos apenas 3 canais dá errado !!!
time_1 = timeup (CH1_input);   // armazena em time_1 o valor obtido
previous_1 = time_1;
time_2 = timeup (CH2_input);   // a partir da função timeup. O mesmo
previous_2 = time_2;
time_3 = timeup (CH3_input);   // procedimento é repetido para os canais 2 e 3.
previous_3 = time_3;

if(previous_1 - time_1 <= 3)
  time_1 = previous_1;
else if(previous_2 - time_2 <= 3)
  time_2 = previous_2;
else if(previous_3 - time_3 <= 3)
  time_3 = previous_3;
// Leitura feita em aprox 20ms (19720 us);
// Para checar a leitura dos canais, descomentar o código abaixo.
  //Serial.print("CH1 :");
  //Serial.print("    ");
  //Serial.print("CH2 :");
  //Serial.print("    ");
  //Serial.println("CH3 :");
  //Serial.print(time_1);
  //Serial.print("         ");
  //Serial.print(time_2);
  //Serial.print("         ");
  //Serial.println(time_3);

if(time_2 < 145)         // Para que os valores não ultrapassem o valor mapeado nas coordenadas,
  time_2 = 145;          // às variáveis "timer_1" e "timer_2" serão atribuidos os valores limites
else if(time_1 < 145)    // sempre que o valor exceder esses limites, por conta de variações no sinal.
  time_1 = 145;
else if (time_2 > 261)
  time_2 = 261;
else if(time_1 > 261)
  time_1 = 261;
//else if(time_1 > 1000 || time_2 < 1000 || time_1 < 10 || time_2 < 10)
//  break;

coord_X = map(time_1, 145, 261, -255, 255);
coord_Y = map(time_2, 145, 261, -255, 255);

//------------Controle de coordenadas(DEVE SER OTIMIZADO)--------------
// O funcionamento desse sistema de coordenadas, assemelha-s
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

//------------------------------------
// Descomente para testar os valores enviados às portas.
//Serial.print("MOTOR_L:");
//Serial.print("    ");
//Serial.println("MOTOR_R:");
//Serial.print(L);
//Serial.print("         ");
//Serial.println(R);

if(R < 0){
  digitalWrite(right_reverse,HIGH);
  digitalWrite(right_forward,LOW);
  analogWrite(motor_r, R*-1);
}else if(R > 0){
  digitalWrite(right_reverse,LOW);
  digitalWrite(right_forward,HIGH); 
  analogWrite(motor_r, R);
}
if(L > 0){
  digitalWrite(left_reverse,LOW);
  digitalWrite(left_forward,HIGH);
  analogWrite(motor_l, L);
}if(L < 0){
  digitalWrite(left_reverse,HIGH);
  digitalWrite(left_forward,LOW);
  analogWrite(motor_l, L*-1);
}
if(L == 0 || R == 0){
  digitalWrite(left_reverse,LOW);
  digitalWrite(left_forward,LOW);  
  digitalWrite(right_reverse,LOW);
  digitalWrite(right_forward,LOW);  
}
}