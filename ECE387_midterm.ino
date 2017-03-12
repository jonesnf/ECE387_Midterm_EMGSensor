#include <Servo.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//Pins for I/O
Servo myServo;
int aPIN = 0;                           //analog pin to be used (A0)
int pos = 0;


//Interrupt values 
volatile int muscData;
volatile boolean ADC_DONE;
volatile int oldVal;
volatile int newVal = 0;
boolean adcStarted;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");
  myServo.attach(9);
  
  ADMUX = (1 << REFS0) | (aPIN & 0x07);
  ADCSRA |= (1 << ADSC) | (1 << ADIE);
  
}

ISR(ADC_vect){
  byte low, high;

  oldVal = newVal;

  low = ADCL;
  high = ADCH;  
  muscData = low | (high << 8);

  newVal = muscData;
  
  ADC_DONE = true;
  
}

void loop() {
  // put your main code here, to run repeatedly:

  if(ADC_DONE){

      adcStarted = false;

      
      Serial.print("Old data:   ");
      Serial.println(oldVal);
      Serial.print("New data:   ");
      Serial.println(muscData);
      

      if(muscData > oldVal){
           pos+= 1;
           Serial.println("move!");
          myServo.write(90) ; 
           delay(100);
        
      }else{
        pos-=1;
        Serial.println("don't move!");
        myServo.write(180);
        delay(100);
      }
      
      
      ADC_DONE = false;
    delay(1000);
    
    }

   if(!adcStarted){

      ADCSRA |= bit(ADSC) | bit(ADIE);
      adcStarted = true;   
    
    }    

}















