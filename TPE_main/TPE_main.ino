/*
 * 10 == CS#
   11 == SI
   13 == SCK
   2 == Debimetre
   A5 == IR
   5 == Led verte
   6 == Led rouge
   A1 == Led mode 1
   A2 == Led mode 2
   A3 == Led mode 3
   4 == Electrovanne
   A0 == Bouton
 */

#include <SPI.h>

//Déclaration des entrées
const byte hallsensor = 2, btn = A0;
byte pin_in[] = {hallsensor, btn};

//Déclaration des sorties
const byte LR = 6, LV = 5, LM1 = A1, LM2 = A2, LM3 = A3, van = 4;
byte pin_out[] = {LR, LV, LM1, LM2, LM3, van};

const byte slaveSelect = 10, maxCount = 99;
const byte timerClock = 2000, timer = 10, tempOn = 3, orange = 30, red = 60, redb = 99, eco = 35, hum = 35, rin = 30;
 
int millis0 = 0, calc, time, tps, time_mode, Clock, mode = 0, diffclock, access = 1;
int mbE = 0, state = 0, md, ir_value, distance, a, i, c, d = 0, e = 0, f = 0, conso, conso_temp, confirm, last, rcpt, user_last, user_secure, user;

int ir = A5;

volatile int NbTopsFan; //measuring the rising edges of the signal

void setup(){
  SPI.begin();   // initialize SPI
  pinMode(slaveSelect, OUTPUT);
  digitalWrite(slaveSelect,LOW);  // select slave
  sendCommand(12,0);  // normal mode (default is shutdown mode);
  sendCommand(15,1);  // Display test on
  sendCommand(10,2);  // set medium intensity (range is 0-15)
  sendCommand(11,2);  // 7221 digit scan limit command
  sendCommand(9,255); // decode command, use standard 7-segment digits
  digitalWrite(slaveSelect,HIGH);  // deselect slave

  for(byte a = 0; a < sizeof(pin_in); a++){
    pinMode(pin_in[a], INPUT);
  }

  for(byte a = 0; a < sizeof(pin_out); a++){
    pinMode(pin_out[a],OUTPUT);
  }
  digitalWrite(btn, HIGH);
  
  digitalWrite(LV, LOW);
  digitalWrite(LR, LOW);

  attachInterrupt(0, rpm, RISING); //and the interrupt is attached

  delay(2000);
  digitalWrite(slaveSelect,LOW);  // select slave
  sendCommand(15,0);  // Display test off
  sendCommand(12,0);  // normal mode (default is shutdown mode);
  digitalWrite(slaveSelect,HIGH);  // deselect slave
  displayNumber(0);
  Serial.begin(9600);
}

void loop(){
//Variables à valeur changeante
  time = millis()/1000;
  md = digitalRead(btn);

//Appel de fonction
  modeSelect();
  modeLed();

//Ajoute un delai avant coupure
  if(user_detect()){
      user_secure = time;
      user = 1;
  }else if(user_secure + 1 <= time){
      user = 0;
  }

//Block principal
  if(user){
    attachInterrupt(0, rpm, RISING);
    tps = time;    
    if(access){
      digitalWrite(van, HIGH);
    }else{
      digitalWrite(van, LOW);
    }
    if(state == 0){
      NbTopsFan = 0;
      conso = 0;
      turn_On();
      displayNumber(calc);
      state = 1;
    }
  }else{
    detachInterrupt(0);
    digitalWrite(van, LOW);
    if(state == 1){
      if(time - tps > timer){
        turn_Off();
        last = 0;
        rcpt = 0;
        state = 0;
        tps = 0;
        calc = 0;
        d = 0;
        e = 0;
        f = 0;
        c = 0;
        rcpt = 0;
        access = 1;
        Serial.println(conso);               
      }
    }
  }
  
//Block des modes  
  if(state == 1){
   switch(mode){
    case 0: 
        calc = conso;
        displayNumber(calc);
        tricolor(orange, red, redb);
    break;
    case 1:
        calc = (eco - conso);
        displayNumber(calc);
        tricolor(eco/3*2, eco/3, 2);
        if(eco/2-2 < calc && eco/2+2 > calc && d == 0){
          detachInterrupt(0);
          if(f == 0){
            e = time;
            f = 1;
          }   
          access = 0;
          displayBlink();
          if(time - e > 6 && d == 0){
            attachInterrupt(0, rpm, RISING);
            access = 1;
            d = 1;
            displayLight();
          } 
        }
        if (calc <= 0){
            access = 0;
        }
        break;
    case 2:
      if(c <= 50){
        mode = 0;
        c = 0;
      }
      if(rcpt == 0){
        Serial.println(-1);
        last = Serial.parseInt();
        c++;
      }
      if(last > 0){
        rcpt = 1;
        if(last > hum){
          calc = (last*0.9 - conso);
          tricolor(last*0.9/3*2, last*0.9/3, 2);
          }else{
          calc = (hum - conso);
          tricolor(eco/3*2, eco/3, 2);
        }
        if (calc <= 0){
          access = 0;
        }
        displayNumber(calc);   
      }
    break;
    }
  }
}

// Fonction d'affichage d'un nombre
void displayNumber( int number){
  if(number<=maxCount && number >= 0){
    for (int i = 0; i < 2; i++)
    {
      byte character = number % 10;  // get the value of the rightmost decade
      // send digit number as command, first digit is command 1
      sendCommand(2-i, character);
      number = number / 10;
    }
  }else if(number>maxCount){
    sendCommand(2, 9);
    sendCommand(1, 9);
  }
}

//Fonction pour faire clignoter le digits
void displayBlink(){
  if(clock()){
    digitalWrite(slaveSelect,LOW);
    sendCommand(10,2);
    digitalWrite(slaveSelect,HIGH);
  }else{
    digitalWrite(slaveSelect,LOW);
    sendCommand(10,0);
    digitalWrite(slaveSelect,HIGH);
  }
}

//Fonction de communication avec le digits
void sendCommand( int command, int value){
  digitalWrite(slaveSelect,LOW);
  SPI.transfer(command);
  SPI.transfer(value);//test
/**/
  digitalWrite(slaveSelect,HIGH);
}

//Fonction qui gère la led tricolore
void tricolor(int born1, int born2, int born3){
  if(born1 < born3){
    if (calc < born1){
      color_set(254, 0);
    }else if (calc >= born1 && calc < born2){
      color_set(127, 127);
    }else if (calc >= born2 && calc < born3){
      color_set(0, 254);
    }else{
      led_blink();
      displayBlink();
    }  
  }else{
    if (calc > born1){
      color_set(254, 0);
    }else if (calc <= born1 && calc > born2){
      color_set(127, 127);
    }else if (calc <= born2 && calc > born3){
      color_set(0, 254);
    }else{
      led_blink();
      displayBlink();
    }   
  }
}

//Fonction qui definie la couleur de led
void color_set(int lv, int lr){
  analogWrite (LV, lv);
  analogWrite (LR, lr);
}

//Fonction qui clignoter la led
void led_blink(){
  digitalWrite(LV, LOW);
  if(clock()){
    analogWrite(LR, 255);
  }else{
    analogWrite(LR, 25);
  }
}

//Fonction qui allume les témoins
void turn_On(){
  digitalWrite(slaveSelect,LOW);
  sendCommand(12,1);
  sendCommand(10,2);
  digitalWrite(slaveSelect,HIGH);
  color_set(255,0);
}

//Fonction qui éteint les témoins
void turn_Off(){
  digitalWrite(slaveSelect,LOW);
  sendCommand(12,0);
  digitalWrite(slaveSelect,HIGH);
  color_set(0,0);
  digitalWrite(LM1, LOW);
  digitalWrite(LM2, LOW);
  digitalWrite(LM3, LOW);
}

//Fonction de détéction de la consomation (attacher à un interupt)
void rpm (){
  NbTopsFan++;
  conso_temp = NbTopsFan / 10;
  if(conso != conso_temp){
    conso = conso_temp;
  }
}

//Générateur à pulsation
int clock(){
  diffclock = millis() - millis0;
  if (diffclock < timerClock ){
    return 1;
  }else if (diffclock < timerClock*2 ){
    return 0;
  }else{
    millis0 = millis();
  }
} 

//Fonction de détéction d'utilisateur
int user_detect(){
  a = 0;
  for(i = 0; i <= 5; i++){
    ir_value = analogRead(ir);
    distance = 4800/(ir_value - 20);
    if(distance > 10 && distance < 30){
      a++;
    }
  }
  if(a == i){
    return 1;
  }else{
    return 0;
  }
}

//Detection du mode
void modeSelect(){
  if ((mbE == 0) && (md == 0)){
    if(time >= time_mode){
      confirm = 0;
    }
    if(state){
      confirm = 1;
    }
    if(confirm){
      mode++;
      if (mode == 3){
          mode = 0;
      }
    }
    mbE = 1;
    time_mode = time + tempOn;
    confirm = 1;
    modeLed2();
  }else if (md == 1){
    mbE = 0;
  }
}

//Gestion des leds mode
void modeLed(){
  if(state){
    modeLed2();
  }else if(state == 0 && time >= time_mode){
    digitalWrite(LM1, LOW);
    digitalWrite(LM2, LOW);
    digitalWrite(LM3, LOW);
  }
}

void modeLed2(){
  switch(mode){
    case 0:
      digitalWrite(LM1, HIGH);
      digitalWrite(LM2, LOW);
      digitalWrite(LM3, LOW);
    break;
    case 1:
      digitalWrite(LM1, LOW);
      digitalWrite(LM2, HIGH);
      digitalWrite(LM3, LOW);
    break;
    case 2:
      digitalWrite(LM1, LOW);
      digitalWrite(LM2, LOW);
      digitalWrite(LM3, HIGH);
    break;
  }
}

void displayLight(){
  digitalWrite(slaveSelect,LOW);
  sendCommand(10,2);
  digitalWrite(slaveSelect,HIGH);
}