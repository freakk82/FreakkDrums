//*******************************************************************************************************************
// 
//      FREAKK MIDI DRUMS
//			
//*******************************************************************************************************************


//*******************************************************************************************************************
// Defines			
//*******************************************************************************************************************

#define midichannel	0                              // MIDI channel from 0 to 15 (+1 in "real world")
#define DEF_MAX_VEL 1023
#define NOTE_ON 144
#define NOTE_OFF 128
#define LED 13

#define THRESHOLD 6
#define READ_TIME 4   // milliseconds
#define TAIL_TIME 22   // milliseconds

// pin modes
#define MODE_READ 0
#define MODE_TAIL 1
#define MODE_STANDBY 2


//*******************************************************************************************************************
// Global Variables			
//*******************************************************************************************************************

// unsigned char PadNote[6] = {52,16,66,63,40,65};         // MIDI notes from 0 to 127 (Mid C = 60)
unsigned char PadNote[6] = {36,38,22,63,40,65};         // MIDI notes from 0 to 127 (Mid C = 60)
int PadThreshold[6] = {
THRESHOLD, THRESHOLD, THRESHOLD, THRESHOLD, THRESHOLD, THRESHOLD};           // Minimum Analog value to cause a drum hit
boolean VelocityFlag  = true;                           // Velocity ON (true) or OFF (false)
boolean boostActive = true;
int boost = 1.5;
int maxVelocity[6] = {DEF_MAX_VEL,DEF_MAX_VEL,DEF_MAX_VEL,DEF_MAX_VEL,DEF_MAX_VEL,DEF_MAX_VEL};



//*******************************************************************************************************************
// Internal Use Variables			
//*******************************************************************************************************************

boolean activePad[6] = {
  0,0,0,0,0,0};                   // Array of flags of pad currently playing
int PinPlayTime[6] = {
  0,0,0,0,0,0};                     // Counter since pad started to play

unsigned char status;

int pin = 1;     

int value = 0;
int hitSend[6] = {0,0,0,0,0,0};
int maxVal[6] = {0,0,0,0,0,0};
int startTime[6] = {0,0,0,0,0,0};
int readTime[6] = {0,0,0,0,0,0};
int elapsedTime[6] = {0,0,0,0,0,0};
int pinStatus[6] = {MODE_STANDBY,MODE_STANDBY,MODE_STANDBY,MODE_STANDBY,MODE_STANDBY,MODE_STANDBY};

//*******************************************************************************************************************
// Setup			
//*******************************************************************************************************************

void setup() 
{
  Serial.begin(115200);
  pinMode(LED, OUTPUT);     // connect to the serial port 115200

  //CalibrateMaxVelocity(0);
  //delay(500);
  //CalibrateMaxVelocity(1);

}

//*******************************************************************************************************************
// Main Program			
//*******************************************************************************************************************

void loop() 
{
  for(pin=0; pin<2; pin++){

    value = analogRead(pin);

    /* --- HIT DETECTION --- */
    if( value > PadThreshold[pin] ) {
      if(pinStatus[pin] == MODE_STANDBY) {
        startTime[pin] = millis();
        pinStatus[pin] = MODE_READ;
        maxVal[pin] = value;
      }
    }// #Hit Detection

    /* --- PEAK DETECTION --- */
    if(pinStatus[pin] == MODE_READ) {
      if(value > maxVal[pin]) maxVal[pin] = value;
      elapsedTime[pin] = millis() - startTime[pin];
      if(elapsedTime[pin] > READ_TIME) { // end of peak detection window
        hitSend[pin] = (int)( boost * 127.0 * maxVal[pin]/ maxVelocity[pin] );  // balanced velocity
        if (hitSend[pin] >127) hitSend[pin] = 127;
        //hitSend[pin] = (int)( 127.0 * maxVal[pin] / DEF_MAX_VEL);  // unbalanced
        //Serial.println(maxVal[pin]); // debug: output peak value
        MIDI_TX(NOTE_ON,PadNote[pin],hitSend[pin]); // send NOTE_ON
        pinStatus[pin] = MODE_TAIL;
        startTime[pin] = millis();
      }
    } // #Peak Detection

    /* --- TAIL REJECTION --- */
    if(pinStatus[pin] == MODE_TAIL) {
      elapsedTime[pin] = millis() - startTime[pin];
      if(elapsedTime[pin] > TAIL_TIME) {
        MIDI_TX(NOTE_OFF,PadNote[pin],0);
        pinStatus[pin] = MODE_STANDBY;
        maxVal[pin] = 0;
      }
    } // #Tail Rejection
  }
} // #Main Loop


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

void CalibrateMaxVelocity(int pin)
{
  boolean done = false;
  boolean peakFound = false;
  int start = 0;
  int elapsed = 0;
  int val = 0;
  int maxV = 0;
  digitalWrite(LED, HIGH);

  while(!done){
    // HIT DETECTION
    val = analogRead(pin);
    maxV = val;
    if(val > PadThreshold[pin]){
      start = millis();
      // PEAK DETECTION
      do{
        val = analogRead(pin);
        if(val > maxV) maxV = val;
        elapsed = millis() - start;
        if(elapsed > READ_TIME){ // end of peak detect window
          maxVelocity[pin] = maxV;  // balanced velocity
          peakFound = true;
        }
      } 
      while(!peakFound);
      done = true;
    }

  }
  //flash led to signal calibration
  FlashLed(4,200);


} // #CalibrateMaxVelocity


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



