 
//DECLARE FUNCTIONS
void motors();
int selector(int);
void shoot(int);
void solenoid();



//Sensors:
//On/Off switch
const int   selectorSwitchPin =   7;
int selectorSwitch;  //analog voltage divider

const int   potentiometerPin =    6;
double potentiometer;    //analog

const int   firingTriggerPin =    8;
int firingTrigger = 0; //digital          PULL DOWN PIN TO GROUND

const int   revTriggerPin =     9;
int revTrigger = 0;    //digital          PULL DOWN PIN TO GROUND


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


//to designate pin types:
//  pinMode (pinVariable, INPUT_PULLUP or OUTPUT)
//to output signal:
//  digitalWrite (thingPin, thing)
//VALUE is implied if no suffix

//okay, here's to future me:
/*  
 *   here was what the plan was, Ben. We have two outputs, the motors and the solenoid.
 *   The motors run off a PWM signal, so you can make it shoot a lot less hard and not hurt
 *   or so you can make it shoot really hard and definitely hurt
 *   The solenoid pulses depending on a "mode"
 *   you've got a single fire, burst fire, and full auto mode.
 *   
 *   on to the stuff you don't understand:
 *   
 *   There's a few functions we made to make our lives easier and the lives
 *   of any other poor souls reading this shit a nightmare.
 *   We have:
 *      sensorRead()- this one didn't work. It was supposed to read sensors and write to
 *                    global variables, but it wasn't having fun.
 *      motors()- this reads the pot, remaps the input from 0-1023, to 50% - 100%
 *                also it runs the motors. neat stuff.
 *                we also added braking on 25 Feb 2021 to digital pin 11
 *                we had to wait 5 ms so that we dont short the battery (done in loop)
 *      selector(int)- this reads the selector switch (a voltage divider)
 *                     the voltage divider thing is actually pretty rad.
 *                     It acts like a potentiometer with discrete steps in resistance.
 *                     The circuit also allows us to light up different lights **without*
 *                     adding more wires.
 *      shoot(int)- this controls when solenoid() is called. We didn't put this one in
 *                  the main (I guess its loop() with arduino, huh?) because it looked
 *                  like a lot and big chunks scare us.
 *      solenoid()- the star of the show. This function has three distinct phases
 *                  Phase 1: Off timer. 
 *                  Phase 2: On timer.
 *                  Phase 3: Off and reset.
 *   
 *   We wanted to lock out the user from firing the solenoid two times at once.
 *   That way, if they're a dumb dummy dummybrain and mash the shit out of the trigger
 *   we dont confuse the microcontroller.
 *   We made the lockout system, here's how it works:
 *      We made a variable called lockout. If lockout is hi, or = 1, we're "locked out"
 *      otherwise, (aka when its low, or = 0) we aren't locked out.
 *      We initialize the variable lockout to 0
 *      If lockout is hi, shoot() cant be called (i think)
 *      Lockout is set hi whenever a loop in shoot() finishes
 *      Lockout is set low after the timer when:
 *        Trigger is no longer pulled
 *        The last loop in shoot() finished
 *        and we were already in lockout.
 *      If lockout was low here AND trigger isnt pulled, we reset pastTime
 *      
 *      
 *      20 Mar
 *        Added back braking, it works well
 *        Changed motor mapping to use variables
 *        something smells burnt
 *        motor output set to high for testing
 *        issue is that Vg < Vs
 *        Load is on transistor source, moving to transistor drain
 *        Voltage drops before "drain" pin
 *        Source will be already grounded
 *        
 *      23 Mar
 *        I remade the circuit
 *        I fucking broke a mosfet
 *        
 *      17 Apr
 *        Last time, I burned my finger on the brake mosfet. 
 *        I guess the gate voltage jumped up when I turned on the motors.
 *        I added code to reduce the PWM frequency from 490.20 Hz to 30.64 Hz
 *        
 *      9 May
 *        Long time since update.
 *        I remade the circut again.
 *        Now it has a socket for the arduino, to make replacements and uploads easier.
 *        I removed braking since it added more that could go wrong and I have not yet fully gotten motors working
 *        I can turn the motors now. 
 *        The power switch transistor was on the high side. 
 *        I should've had it control the ground line instead.
 *        I set PWM to a higher frequency.
 *        PWM, manual or automatic is not working.
 *        Maybe the constant change in voltage is messing with the input voltage to the Arduino?
 *        I would have to manually PWM with a really low frequency
 *        
 *      27 May
 *        Added the voltage regulator. 
 *        Bypassed the 5V out on board for regulator voltage
 *        Changed Pulldown resistor from 2.2k to 1M
 *        Changed to linear 10k Pot
 *        PWM alone works as low as 50/255, but in code it tends to stop working
 *          why is this?
 *        Maybe all my counters are slowing down processes
 *        I also think I burned out both the motors and board
 *        Two jams held for too long
 *        Board no longer outputting to pin 3.
 *        
 *      30 May
 *        The PWM works in isolation.
 *        Trying to now *lower* the PWM frequency
 *        It worked fine on the 150/255 output test
 *        Loweing to 50/255 output
 *        Works at 50/255 output
 *        Motors start spinning very slowly, but I don't want to make a motor starter circuit
 *        motorOut variable isn't working
 *        hard-coding the motorout values
 *        fuck it we're keeping the manual coding. fuck the variables
 *        
 *      31 May 2021
 *        Here's the plan for tomorrow:
 *        I'm gonna turn on the camera, point it at the nerf gun,
 *        then test to see if the trigger and voltage multiplier work right
 *        i should test if the LED works right now just in case, though.
 *        
 *      28 June 
 *        IT WORKS!
 *        I got full auto firing done.
 *        I strengthened the return spring then tweaked the solenoid on time/frequency.
 *        I added a portion at the far bottom of void loop() where I had the solenoid turn
 *        off if in full auto mode and the cycle was finished.
 *        It was easier to do this than to integrate full auto into the "lockout" system.
 *        Not the best solution, but I'm fully done now.
 *        I have to buy a separate battery now, though. Voltage multiplier won't work.
 *        
 *      29 June
 *        Second Batteries came in today.
 *        Everything works perfectly.
 *        I tuned the frequency and on time a little more today.
 *        Keeping it down to 10Hz and a 30ms on time.
 *        I added a line of code to reset "i" to 0 every time the rev is
 *        released.
 *        I had an issue where I could Rev > Burst > Release Rev and the
 *        next time you rev, it runs the three burst.
 *        Hopefully that fixes it.
 *        No other major bugs or hiccups.
 *        Might increase frequency and lower on time of solenoid.
 */


void setup() {

  Serial.begin(19200);

  pinMode(selectorSwitchPin, INPUT_PULLUP);
  pinMode(potentiometerPin, INPUT_PULLUP);
  pinMode(firingTriggerPin, INPUT);
  pinMode(revTriggerPin, INPUT);

  pinMode (motorOutPin, OUTPUT);
  pinMode (solenoidOutPin, OUTPUT);
//TCCR2B = TCCR2B & B11111000 | B00000001; // for PWM frequency of 31372.55 Hz
//TCCR2B = TCCR2B & B11111000 | B00000100; // for PWM frequency of 490.20 Hz (The DEFAULT)
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
/*
Serial.print ("\n \n firing is ");                  //keeping this one in.
Serial.print (firing);
Serial.print ("\n pot: ");
  Serial.print (potentiometer);                  */                         
//Serial.print (digitalRead(revTriggerPin));
  if (revTrigger == 1) {                //only while trigger is pulled
    motors();                           //PWM motors
    if (firing == 1)                    //and while firing is toggled on
    {shoot(firingMode);}                //call shoot()
//    m_pastTime = millis();
  }
   else {analogWrite (motorOutPin, 0); m_pastTime = millis(); i = 0;}
/*  else {
    analogWrite (motorOutPin, 0);       //if trigger isnt pulled, immediately turn off motor power
    if (millis() - m_pastTime < 50 )     //wait 5 ms before activating braking
    {digitalWrite (11, 1);}             //brake
    //Serial.print (" \n brake \n ");
  }*/

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
//  Serial.print ("\n motors \n");

//if ((millis() - m_pastTime) < 50) {analogWrite (3, 255);}    //motor starter

//else {
  potentiometer = analogRead(potentiometerPin);              //reads potentiometer
//  digitalWrite (11, 0);                                    //turns off braking
  if (potentiometer > potLower) {                            //if potentiometer
    if (potentiometer < potUpper)                            //within deadzone
    { 
 // Serial.print ("\n remap ");
 // Serial.print( (potentiometer - 250)*127/(800-250) + 128);
  
  //motorOut = ( (potentiometer - potLower)*(255-20)/(potUpper-potLower) + 20.0);       //WHY THE FUCK DOES IT NEED A DIVIDE BY 10
 analogWrite (3, ( (potentiometer - potLower)*(255-60)/(potUpper-potLower) + 60.0) );   //Idk what fixed it but no longer need a /10
  } //map from 0-1024 analogIn to 128-255 analogOut
  }
  if (potentiometer <= potLower)                             //lower deadzone, 50% duty cycle
    {//motorOut =  50; 
    analogWrite (3, 60);}
  if (potentiometer >= potUpper)                             //upper deadzone, 100% duty cycle
    {//motorOut = 255; 
    analogWrite (3, 255);}


  Serial.print ("\n pot: ");
  Serial.print (potentiometer);
  Serial.print ("\n mO: ");
  Serial.print (motorOut); 
  //analogWrite (3, motorOut);                  //PWM signal remapped to 50% Duty Cycle at lowest
  
  //digitalWrite (motorOutPin, 1);
//  }
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
    //digitalWrite (3, 0);
  }
  else if (timeNow - s_pastTime < s_offTime + s_onTime) {
    solenoidOut = HIGH;                                 //If timer is no longer less than off timer
    //digitalWrite (3, 1);
  //  Serial.print ("\n \n sol on \n \n");
  }
  else {                                                //If timer is greater than the off+on timer
    solenoidOut = LOW;    //Set output to OFF
    //digitalWrite (3, 0);
    s_pastTime = millis();    //Reset timer
  //  Serial.print ("\n \n i + 1 \n \n");
    i = i + 1;
  }
    digitalWrite (solenoidOutPin, solenoidOut);         //sets outputValue to be used on the pin
}
