/*
   Blink
   Turns on an LED on for one second, then off for one second, repeatedly.
  
   This example code is in the public domain.
  */
  

class LED
{
    bool _Off;
    int _Pin;
    bool _Flash;
    int _Delay;
    unsigned long _Previous;
    public:
    LED(int pin) : _Pin(pin), _Off(true), _Flash(false), _Delay(0)
    {
           pinMode(_Pin, OUTPUT); 
           digitalWrite(_Pin,LOW);  
    }
    
    void toggle()
    {
      digitalWrite(_Pin, _Off ? HIGH : LOW);   // turn the LED on (HIGH is the voltage level) 
      _Off = !_Off;      
    }
    
    void freeRun(int delayTime, bool state)
    {
      _Flash = state;
      _Delay = delayTime;
      _Previous = millis();
    }
    
    void display()
    {
      unsigned long now = millis();
         if (now - _Previous > _Delay)
       {
          digitalWrite(_Pin, _Off ? HIGH : LOW);   // turn the LED on (HIGH is the voltage level) 
          _Previous = now;
          _Off = !_Off;
       }
    }
    
};
// Pin 13 has an LED connected on most Arduino boards.
// give it a name:
//int led = 9;
//bool off = true;
//unsigned long previous = millis();

LED led(9);

// the setup routine runs once when you press reset:
void setup() {  
   // initialize the digital pin as an output.
//   pinMode(led, OUTPUT); 
//   digitalWrite(led,LOW); 
led.freeRun(200, true);
}

// the loop routine runs over and over again forever:
void loop() 
{
//   unsigned long now = millis();
//   if (now - previous > 500)
//   {
//      digitalWrite(led, off ? HIGH : LOW);   // turn the LED on (HIGH is the voltage level) 
//      previous = now;
//      off = !off;
//   }

7
}
