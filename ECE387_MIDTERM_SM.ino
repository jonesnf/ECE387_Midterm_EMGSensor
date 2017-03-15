#include <Servo.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define ST_INIT      0xF                       //Initializing emg sensor. First go 
#define ST_RST       0xE                       //Reset
#define ST_CAL       0x1                       //State Cal - calibration sate  
#define ST_IDLE      0x2                       //Adding idle state as a no-op 
#define ST_GO        0x3                       //State 1 - increase in muscle activity, assuming walking if coming from idle state
#define ST_STRIDE    0x4                       //State 2 - Mid stride - serves as reference point
#define ST_PASS      0x5                       //State 3 - Pass position state - if two legs were implemented, 
                                                  //this would be where the free foot passes the foot bearing the load
                                              


#define aPIN         0x0                       //analog pin to be used (A0)


//Pins for I/O
Servo myServo;
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

//Values for implementing millis() timer
int calibTime = 3000;
long currentTime = 0;
long resetTime = 0;
long resetInterval = 0;


void setup() {
  Serial.begin(9600);
  Serial.println("Initializing...");
  myServo.attach(9);
  
  ADMUX = (0 << REFS1)                    
        | (1 << REFS0)                      //Here we are selecting what reference we will be using (See datasheet for refences)
        | (aPIN & 0x07);                    //Selecting A0 

        
  ADCSRA |= (1 << ADSC) | (1 << ADIE);      //Setting the Interrupt En. and Start conversion bits in our ADCSRA register

  STATE = ST_INIT;                           //Setting the state to calibrate, when the emg sensor boots up it outputs high for a couple seconds
  
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

     //Reset checking
     if(muscData == 0 && (STATE != ST_INIT || STATE != ST_RST)){
        resetTime = millis();                 // time we reset               
        resetInterval = resetTime + 3000;
        STATE = ST_RST;
        
     }
     
     
     switch(STATE){

      //14
      case ST_RST:
            resetTime = millis();
            (resetTime >= resetInterval) ? STATE = ST_IDLE : ST_RST;
            break;
      //15
      case ST_INIT:
            currentTime = millis();
            (currentTime >= calibTime) ? STATE = ST_IDLE : ST_INIT;
            break;
      //1
      case ST_CAL:
      
           //do nothing.  need to hold position here while EMG sensor calibrates
           resetTime = millis();
           Serial.print(F("Value: "));
           Serial.println((muscData));
           (resetTime >= resetInterval) ? STATE = ST_IDLE : ST_CAL;
           break;
      //2     
      case ST_IDLE:
      
           myServo.writeMicroseconds(1500);
           Serial.print(F("Value: "));
           Serial.println((muscData));
           (muscData > oldVal && (muscData - oldVal >= 50)) ? STATE = ST_GO : STATE = ST_IDLE;           
           break;
      //3     
      case ST_GO:

           Serial.print(F("Value: "));
           Serial.println((muscData));
           //drive = map(muscData, 0, 700, 120, 180);       //Scaling down our value from our EMG sensor from [0-700] to [0-180] ( > 90 to lift leg )
           myServo.write(160);
           delay(200);
           myServo.writeMicroseconds(1500);
           STATE = ST_STRIDE;           
           break;
      //4     
      case ST_STRIDE: 
      
           //mid stride now, no need to do anything except check data
           Serial.print(F("Value: "));
           Serial.println((muscData));
           //(muscData < oldVal && ( oldVal - muscData >= 75) && STATE == ST_STRIDE) ? STATE = ST_PASS : STATE = ST_STRIDE; 
           (muscData < oldVal && ( oldVal - muscData >= 75)) ? STATE = ST_PASS : STATE = ST_STRIDE; 
           break;
      //5
      case ST_PASS: 

           Serial.print(F("Value: "));
           Serial.println((muscData));
          // drive = map(muscData, 0, 700, 0, 60);       //Scaling down our value from our EMG sensor from [0-700] to [0-180] ( > 90 to lift leg )
           myServo.write(20);
           delay(200);
           myServo.writeMicroseconds(1500);
           STATE = ST_IDLE;           
           break;

      default:
           break;
     
     }
      delay(700);
      Serial.println(STATE);
      adcStarted = false;
    
    }

   if(!adcStarted){

      ADCSRA |= (1 << ADSC) | (1 << ADIE);
      adcStarted = true;   
      rxReady = true;
    
    }    

}















