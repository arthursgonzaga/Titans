#define duty_max 140
#define duty_min 140 
#define duty_zero 0

#define sensor_left A1
#define sensor_right A0
#define motor_left 10
#define motor_right 9

int IR_right;
int IR_left;
char rot = 'n';

void leitura_sensores(){

     IR_right = digitalRead(A0);
     IR_left = digitalRead(A1);

}

void setup(){

Serial.begin(9600);
pinMode(sensor_left, INPUT_PULLUP);
pinMode(sensor_right, INPUT_PULLUP);
delay(2000);

}

void loop(){

  leitura_sensores();
  
  /*Serial.print(IR_left);
  Serial.print(" ");
  Serial.println(IR_right);*/
  
  if(!IR_left && !IR_right){
    
      if(rot == 'l'){
        
        rot = 'n';
        analogWrite(motor_left, duty_min);
        analogWrite(motor_right, duty_zero);
        delay(4);  
        
      }
      
      if(rot == 'r'){
        
        rot = 'n';
        analogWrite(motor_left, duty_zero);
        analogWrite(motor_right, duty_min);
        delay(4);
      
      }
    
        //analogWrite(motor_left, duty_zero);
        //analogWrite(motor_right, duty_zero);
  }
  else if(!IR_left && IR_right){
  
        rot = 'l';
        analogWrite(motor_left, duty_min);
        analogWrite(motor_right, duty_zero);
        delay(4);
  
  }
  else if(IR_left && !IR_right){
  
        rot = 'r';
        analogWrite(motor_left, duty_zero);
        analogWrite(motor_right, duty_min);
        delay(4);
  
  }else{
      
        analogWrite(motor_left, duty_max);
        analogWrite(motor_right, duty_max);
  
  
  }



}
