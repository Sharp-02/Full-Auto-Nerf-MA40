 
//DECLARE FUNCTIONS
void motors();
int selector(int);
void shoot(int);
void solenoid();



//Sensors:
const int   selectorSwitchPin =   7;
int selectorSwitch;     //analog voltage divider

const int   potentiometerPin =    6;
double potentiometer;   //analog

const int   firingTriggerPin =    8;
int firingTrigger = 0;  //digital          PULL DOWN PIN TO GROUND

const int   revTriggerPin =     9;
int revTrigger = 0;     //digital          PULL DOWN PIN TO GROUND


//Outputs:
const int motorOutPin = 3;
double motorOut;    //digital PWM, analogWrite
const int solenoidOutPin = 10;
int solenoidOut; //digital


//Global Variables:
int solenoidFrequency = 10;
int solenoidPeriod = 1000 / solenoidFrequency;
int burstCount = 1;
unsigned long s_pastTime = 0;
int s_onTime = 35;
int s_offTime = solenoidPeriod - s_onTime;

unsigned long timeNow = 0;
int lockout = 0;
int firing = 0;
int i = 0;

unsigned long m_pastTime = 5;

//deadzones (not read values)
int potUpper = 1015;
int potLower = 850;
int potOffset = 64;



void setup() {

  Serial.begin(19200);

  pinMode(selectorSwitchPin, INPUT_PULLUP);
  pinMode(potentiometerPin, INPUT_PULLUP);
  pinMode(firingTriggerPin, INPUT);
  pinMode(revTriggerPin, INPUT);

  pinMode (motorOutPin, OUTPUT);
  pinMode (solenoidOutPin, OUTPUT);
  
  TCCR2B = TCCR2B & B11111000 | B00000111; // for PWM frequency of 30.64 Hz
}




void loop() {

  selectorSwitch  = analogRead (selectorSwitchPin);   //sensorRead() didn't work and I was too lazy to do something about it
  potentiometer   = analogRead  (potentiometerPin); 
  firingTrigger   = digitalRead (firingTriggerPin);
  revTrigger  = digitalRead (revTriggerPin);        
  motorOut  = digitalRead (motorOutPin);
  solenoidOut   = digitalRead (solenoidOutPin);

  Serial.print ("\n sel: ");
  Serial.print (selectorSwitch);
 
  int firingMode = selector(selectorSwitch);        //select firing mode


  Serial.print ("\n fMode: ");
  Serial.print (firingMode);

  timeNow = millis();
  if (firingTrigger == 1) {                         //if the trigger is pulled
    if (lockout == 0){                              //if trigger pulled and not locked out,fire
      if (firing == 0){                             //AND firing state is low
        s_pastTime = millis();                          //set pastTime to now (for solenoid())
        firing = 1;                                     //toggle firing state on
      }
    }
  }                 
              
  if (revTrigger == 1) {                //only while trigger is pulled
    motors();                           //PWM motors
    if (firing == 1)                    //and while firing is toggled on
    {shoot(firingMode);}                //call shoot()
  }
   else {analogWrite (motorOutPin, 0); m_pastTime = millis(); i = 0;}

  if (digitalRead(firingTriggerPin) == 0){    //if trigger released
  if (lockout == 1){                          //and still "locked out"
    if (timeNow - s_pastTime >= 60){          //if the "lockout timer" cycle is finished
      lockout = 0;                            //set lockout to 0
      i = 0;                                  //and make sure loops in shoot() will work
    }
  }
  else if (firing == 0)                       //but if not locked out, and also not firing (why did I do not firing?)
    {s_pastTime = millis();}                  //set past time to right now
  }                                           //adds redundancy in case I fucked up somewhere above

if (firingMode == 3)  {                       //manually turns off solenoid when in full auto mode: too lazy to fix properly
  if (timeNow - s_pastTime > s_offTime + s_onTime) 
    {digitalWrite (solenoidOutPin, 0);}
  }
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



//turns on motors
void motors() {     
//if ((millis() - m_pastTime) < 50) {analogWrite (3, 255);}    //motor starter
//else {

  potentiometer = analogRead(potentiometerPin);              //reads potentiometer
  if (potentiometer > potLower) {                            //if potentiometer
    if (potentiometer < potUpper)                            //within deadzone
    { 
      //motorOut = ( (potentiometer - potLower)*(255-20)/(potUpper-potLower) + 20.0);       //WHY THE FUCK DOES IT NEED A DIVIDE BY 10
      analogWrite (3, ( (potentiometer - potLower)*(255-60)/(potUpper-potLower) + 60.0) );   //Idk what fixed it but no longer need a /10
  } //map from 0-1024 analogIn to 128-255 analogOut
  }
  
  if (potentiometer <= potLower)                             //lower deadzone, low duty cycle
    {analogWrite (3, 60);}
  if (potentiometer >= potUpper)                             //upper deadzone, 100% duty cycle
    {analogWrite (3, 255);}
}


//checks condition of selector switch (uses voltage divider, like a three position pot)
int selector(int sel)   
{ int  fMode = 1;         //fMode is firingMode

  if (sel <= 333)
  {fMode = 1;}            //low volt = Semi-Auto
  else if (sel <= 800)
  {fMode = 2;}            //mid volt = Burst
  else
  {fMode = 3;}            //hi  volt = Auto
  
  return fMode;
}  


//only called if firing is toggled on
void shoot(int fMode_s) {     
  switch (fMode_s) {
    //switch (3) {
case 1:          //Semi-Auto
  //  Serial.print ("\n \n case 1 \n");
    if (i == 0)
       {solenoid();}
    else {
       i = 0;
       lockout = 1;
       firing = 0;
      }
      
  break;
  
case 2:
 //   Serial.print ("\n case 2 \n");
  if (i < 3)          //if less than three counts for burst
       {solenoid();}
  else {
       i = 0;
       lockout = 1;
       firing = 0;
      }
  break;
  
case 3:
//Serial.print ("\n case 3 \n");
  solenoid();
  i = 0;
  
  if (firingTrigger == 0){
    solenoidOut = 0;
    digitalWrite (solenoidOutPin, solenoidOut);
    firing = 0;
  }
  
  break;
}
}


//pulses solenoid each time function is called
void solenoid() {     
  if (timeNow - s_pastTime < s_offTime) {               //If timer since solenoid turned off is less than the off timer
   // Serial.print ("\n \n sol off \n \n");
    solenoidOut = LOW;    //Set output to OFF
  }
  else if (timeNow - s_pastTime < s_offTime + s_onTime) {
    solenoidOut = HIGH;                                 //If timer is no longer less than off timer
  //  Serial.print ("\n \n sol on \n \n");
  }
  else {                                                //If timer is greater than the off+on timer
    solenoidOut = LOW;    //Set output to OFF
    s_pastTime = millis();    //Reset timer
  //  Serial.print ("\n \n i + 1 \n \n");
    i = i + 1;
  }
    digitalWrite (solenoidOutPin, solenoidOut);         //sets outputValue to be used on the pin
}
