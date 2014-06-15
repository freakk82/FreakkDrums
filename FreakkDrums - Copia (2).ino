//  *****************************************************************************************************************
//  *                                                                                                               *
//  *                                         SpikenzieLabs.com                                                     *
//  *                                                                                                               *
//  *                                           Drum Kit - Kit                                                      *
//  *                                                                                                               *
//  *                                                                                                               *
//  *****************************************************************************************************************
//
//  BY: MARK DEMERS Copywrite 20009
//  April. 2009
//  VERSION: 1.b
//
//  DESCRIPTION:
//  Arduino analog input used to sense piezo drum hits then sent serialy to processing.
//  
//  Required - Hardware:
//  1. Drum kit - kit (From SpikenzieLabs.com)
//  2. Arduino
//
//  Required - Software:
//  1. Serial MIDI converter
//  2. Garage Band, Ableton Live etc ...
//
// LEGAL:
// This code is provided as is. No guaranties or warranties are given in any form. It is your responsibilty to 
// determine this codes suitability for your application.

//*******************************************************************************************************************
// User settable variables			
//*******************************************************************************************************************

#define midichannel	0                              // MIDI channel from 0 to 15 (+1 in "real world")
#define DEF_MAX_VEL 53
#define DEF_CUTOFF 12
#define DEF_PLAYTIME 40
#define LED 13

// unsigned char PadNote[6] = {52,16,66,63,40,65};         // MIDI notes from 0 to 127 (Mid C = 60)
unsigned char PadNote[6] = {36,38,22,63,40,65};         // MIDI notes from 0 to 127 (Mid C = 60)
int PadCutOff[6] = {DEF_CUTOFF, DEF_CUTOFF, DEF_CUTOFF, DEF_CUTOFF, DEF_CUTOFF, DEF_CUTOFF};           // Minimum Analog value to cause a drum hit
int MaxPlayTime[6] = {DEF_PLAYTIME,DEF_PLAYTIME,DEF_PLAYTIME,DEF_PLAYTIME,DEF_PLAYTIME,DEF_PLAYTIME};               // Cycles before a 2nd hit is allowed
boolean VelocityFlag  = true;                           // Velocity ON (true) or OFF (false)
boolean boostActive = true;
int boost = 1;
int maxVelocity[6] = {DEF_MAX_VEL,DEF_MAX_VEL,DEF_MAX_VEL,DEF_MAX_VEL,DEF_MAX_VEL,DEF_MAX_VEL};



//*******************************************************************************************************************
// Internal Use Variables			
//*******************************************************************************************************************

boolean activePad[6] = {0,0,0,0,0,0};                   // Array of flags of pad currently playing
int PinPlayTime[6] = {0,0,0,0,0,0};                     // Counter since pad started to play

unsigned char status;

int pin = 1.7;     
int hitavg = 0;

//*******************************************************************************************************************
// Setup			
//*******************************************************************************************************************

void setup() 
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);     // connect to the serial port 115200
  CalibrateVelocity();
}

//*******************************************************************************************************************
// Main Program			
//*******************************************************************************************************************

void loop() 
{
  
  //for(int pin=0; pin < 2; pin++)
  int  pin=1;  // debug on snare
  {
    hitavg = analogRead(pin);                              // read the input pin

    if((hitavg > PadCutOff[pin]))
    {
      if((activePad[pin] == false))
      {
        if(VelocityFlag == true)
        {
          //hitavg = 127 / ((1023 - PadCutOff[pin]) / (hitavg - PadCutOff[pin]));    // With full range (Too sensitive ?)
          //hitavg = boost * (hitavg / 8) -1 ;          // Upper range
          hitavg = (int)( boost * hitavg * 127.0 / maxVelocity[pin] );  // balanced velocity
          if (hitavg >127) hitavg = 127;
        }
        else
        {
          hitavg = 110;
        }

        MIDI_TX(144,PadNote[pin],hitavg); 
        PinPlayTime[pin] = 0;
        activePad[pin] = true;
      }
      else
      {
        ++PinPlayTime[pin];
      }
    }
    else if((activePad[pin] == true))
    {
       if( ++PinPlayTime[pin] > MaxPlayTime[pin] ) activePad[pin] = false;
      
      PinPlayTime[pin] = PinPlayTime[pin] + 1;
      
      if(PinPlayTime[pin] > MaxPlayTime[pin])
      {
        activePad[pin] = false;
        MIDI_TX(128,PadNote[pin],127); 
      }
      
    }
  } 
}


//*******************************************************************************************************************
// Transmit MIDI Message			
//*******************************************************************************************************************
void MIDI_TX(unsigned char MESSAGE, unsigned char PITCH, unsigned char VELOCITY) 
{
  status = MESSAGE + midichannel;
  Serial.write(status);
  Serial.write(PITCH);
  Serial.write(VELOCITY);
}

void CalibrateVelocity()
{
  boolean done = false;
   digitalWrite(LED, HIGH);
   /*
  // get PadCutOff[pin]
  while(done == false)
  {
    //for(int pin=0; pin < 2; pin++)
    int  pin=1;
    {
      int hit = analogRead(pin);                              // read the input pin
      if((hit > DEF_CUTOFF))
      {
         PadCutOff[pin] = hit;
         // Debug
         
         //Serial.print("MIN: ");
         //Serial.print(hit);
        
         done = true;
      }
    }  
  }

    //flash led to signal calibration
   FlashLed(2,200);
   digitalWrite(LED, HIGH);
   */
  // get MaxVelocity[pin]
  done = false;
  while(done == false)
  {
    //for(int pin=0; pin < 2; pin++)
    int  pin=1;
    {
      int hit = analogRead(pin);                              // read the input pin
      if((hit > PadCutOff[pin]))
      {
         maxVelocity[pin] = hit;
         /*
         // Debug
         Serial.print("MAX: ");
         Serial.print(hit);
         */
         done = true;
      }
    }  
  }
    
    //flash led to signal calibration
   FlashLed(4,200);
}

void FlashLed(int times, int duration)
{
  digitalWrite(LED, LOW);
  delay(duration);
  for (int i=0; i<times; i++)
  {
    delay(duration);
    digitalWrite(LED, HIGH);
    delay(duration);
    digitalWrite(LED, LOW); 
    }
}
