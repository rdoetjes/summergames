
/*
 * SUMMER GAMES 100M DASH RECORD SHATTERING MACHINE!
 * by Callous Coder, 2021 https://youtube.com/callouscoder
 * 
 * This code may be used in any way, shape or shitty form.
 * The code simulates a joystick, which is connected to pins
 * 3, 4, 5, 6, and 7 of PORTD on the Arduino/Atmel
 * The Joystick is active low, so the ports are kept high.
 * We do not use the dreadful fucking shitty digitalWrite, that's for kids!
 * We use the PORTD RAW as ATMEL intended it!
 * 
 */
enum EVENTS { CYCLE, RUN };
static EVENTS EVENT;
static bool isWait = true;


/*
 * This implements a nonblocking delay, when used in a loop.
 * The wait method needs to be called every iteration of the loop.
 * When the wait time is exceed the callback set in callback param will be called and
 * the timer will be reset for the next run.
 *
 * By creating multiple instances you can have multiple independit non-blocking
 * delays in a single thread
 * 
 * It should be in it's a motherfucking own file... but fuck you for even suggesting, that
 * it's a stupid arduino
 */
class Delay {
   unsigned long startTime;
   void (*callback)();
   int delayMs;

  public:
   Delay(int ms, void (*callback)()){
      startTime = millis();

      this->callback = callback;

      setDelay(ms);
   }

   void setDelay(int ms){
     delayMs = ms;
   }

   
   int getDelay(){
     return delayMs;
   }
 
   void wait(){
    
     unsigned long elapsedTime = millis() - startTime;

     if ( elapsedTime >= delayMs){

       callback();

       startTime = millis(); // reset for next delay
     }

   }
};

/*
 * This simulates a turning, clockwise, movement of the joystick.
 * This could potentially be used in the cycling event for SUMMER GAMES II but has not been tested.
 */
int circle(){
  static const int movement[] {4, 12, 8, 24, 16, 48};
  static int s = 0;
  
  int result = movement[s];
  
  s++;
  
  if (s>6) s=0;
  
  return result;
}

/*
 * Simulates the rocking of the joystick left to right, for the 100M dash event in SUMMER GAMES
 */
int leftright(){
  
  static int z = 8;   //static is used so that we can override the z in between function calls

  z = (z == 8)?32:8;

  return z;
}

/*
 * implements the IO to the joystick port, 
 * the shift() method simulates the rotation of the joystick in a clockwise direction
 */
void joystick(){
  PORTD &=0b100000000; // clear all bits exept 7th (that could be status LED that's on)
  
  switch (EVENT) {
    case CYCLE:
       PORTD |= 124-circle(); //now or the current joystick
       break;
    
    case RUN:
      PORTD |= 124-leftright(); //now or the current joystick
      break;
  }
}

/*
 * Parse the control char from the serial console
 */
void parseByte(byte c, Delay *joyDelay){

  // so we can adjust from current speed during game, but 19ms is the fastest the joy stick registers on PAL C64, so 19mss is the default value
  int d = joyDelay->getDelay(); 
  
  if (c == '+') {
     joyDelay->setDelay( (d-=5) );
  }

  
  if (c == '-') {
     joyDelay->setDelay( (d+=5) );
  }

  if (c == 'r') {
     EVENT = RUN;
     isWait = false;
  }

  if (c == 'c') {
     EVENT = CYCLE; //we need to figure out the timing... this doesn't fucking work.. yet...
     isWait = false;
  }

  if ( c == 'f'){
     PORTD &=0b10111111;
     delay(30);
     PORTD |=0b11111100;
  }

  if (c == 'q') {
     isWait = true;
  }
  
}

void setup() {
  Serial.begin(9600);
  DDRD = DDRD | 0b11111110; //set up PORTD as outputs except for serial (we may wanna use that)
  PORTD != 0b01000000;
  delay (1000);
}

void loop() {

  //Setup my none blocking delay object, with a function pointer to joystick function to simulate the "spanking of the salami"
  static Delay joyDelay(19, &joystick);
  
  while(1){
    if (Serial.available() > 0) {
      // read the incoming byte:
      parseByte( Serial.read(), &joyDelay);
    } 

    if (!isWait){
      joyDelay.wait();
    } else{
       PORTD |= 0b11111100;
    }
  }
}
