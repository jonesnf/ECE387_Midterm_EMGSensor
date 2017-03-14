#include <Servo.h>
#include <avr/io.h>
#include <avr/interrupt.h>

//Pins for I/O
Servo myServo;
int aPIN = 0;                           //analog pin to be used (A0)
int pos = 0;
int angle = 0;

//Interrupt values 
volatile int muscData;
volatile boolean ADC_DONE;
volatile int oldVal;
volatile int newVal = 0;
boolean adcStarted;
boolean rxReady;

void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");
  myServo.attach(9);
  
  ADMUX = (0 << REFS1)                    
        | (1 << REFS0)                      //Here we are selecting what reference we will be using (See datasheet for refences)
        | (aPIN & 0x07);                    //Selecting A0 

        
  ADCSRA |= (1 << ADSC) | (1 << ADIE);      //Setting the Interrupt En. and Start conversion bits in our ADCSRA register
  
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
  
  //Checking if we have recieved our data from A0, and we are ready to use it
  if(ADC_DONE && rxReady){

      rxReady = false;

      Serial.print("Old data:   ");
      Serial.println(oldVal);
      Serial.print("New data:   ");
      Serial.println(muscData);

      

      if(muscData > oldVal && (muscData - oldVal) >= 100){
           
        
           angle = map(muscData, 0, 700, 120, 179);
           Serial.print("MOVE! ");
           Serial.println(angle);
           
           myServo.write(angle); 
           delay(100);
           myServo.writeMicroseconds(1500);
           delay(10);
        
      }else{
        angle = map(muscData, 0, 1023, 0, 90);
        Serial.println("don't move!");
        //myServo.write(angle);
        delay(100);
      }
      
      
      ADC_DONE = false;
      delay(10);
      
      adcStarted = false;
    
    }

   if(!adcStarted){

      ADCSRA |= (1 << ADSC) | (1 << ADIE);
      adcStarted = true;   
      rxReady = true;
    
    }    

}















