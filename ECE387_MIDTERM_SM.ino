#include <Servo.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define ST_GO        0x1                       //State 1 - increase in muscle activity, assuming walking if coming from idle state
#define ST_STRIDE    0x2                       //State 2 - Mid stride - serves as reference point
#define ST_PASS      0x3                       //State 3 - Pass position state - if two legs were implemented, 
                                                  //this would be where the free foot passes the foot bearing the load
#define ST_CAL       0x4                       //State Cal - calibration sate  
#define ST_IDLE      0x5                       //Adding idle state as a no-op 

#define aPIN         0x0


//Pins for I/O
Servo myServo;
//int aPIN = 0;                                  //analog pin to be used (A0)
int pos = 0;
int drive = 0;

//Interrupt values and helpers
volatile int muscData;
volatile boolean ADC_DONE;
volatile int oldVal;
volatile int newVal = 0;
boolean adcStarted;
boolean rxReady;

//State Machine 
int STATE = 0;


void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");
  myServo.attach(9);
  
  ADMUX = (0 << REFS1)                    
        | (1 << REFS0)                      //Here we are selecting what reference we will be using (See datasheet for refences)
        | (aPIN & 0x07);                    //Selecting A0 

        
  ADCSRA |= (1 << ADSC) | (1 << ADIE);      //Setting the Interrupt En. and Start conversion bits in our ADCSRA register

  STATE = ST_CAL;                           //Setting the state to calibrate, when the emg sensor boots up it outputs high for a couple seconds
  
}

ISR(ADC_vect){
  byte low, high;

  oldVal = newVal;

  low = ADCL;                              //Must read low value (low register) first so that our high value is locked 
  high = ADCH;                             //If high is read first, our data would be erased 
  muscData = low | (high << 8);            //Combining our data to use later

  newVal = muscData;
  
  ADC_DONE = true;
  
}

void loop() {
  
  //Checking if we have recieved our data from A0, and we are ready to use it
  if(ADC_DONE){

    switch(STATE){
      
      case ST_CAL:
           //do nothing.  need to hold position here while EMG sensor calibrates
           STATE = ST_IDLE;
           break;
      case ST_IDLE:
           myServo.writeMicroseconds(1500);
           (muscData > oldVal && (muscData - oldVal >= 100) && STATE == ST_IDLE) ? STATE = ST_GO : STATE = ST_IDLE;           
           break;
      case ST_GO:
           drive = map(muscData, 0, 700, 120, 180);       //Scaling down our value from our EMG sensor from [0-700] to [0-180] ( > 90 to lift leg )
           break;
           
           
      
      
      
      
      
    
    }
      

/*
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
           delay(250);
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
      */
      adcStarted = false;
    
    }

   if(!adcStarted){

      ADCSRA |= (1 << ADSC) | (1 << ADIE);
      adcStarted = true;   
      rxReady = true;
    
    }    

}















