//#include "arduino.h"
/*
key pad layout
      Column0  Column1  Column2
Row0     1        2        3
Row1     4        5        6
Row2     7        8        9
Row3     *        0        #

Key pad terminals
      1        2        3        4        5        6        7
      Column1  Row0  Column0    Row3  Column2     Row2     Row1
      
      '*' key is used as a cancel button
      '#' key is used as a enter button

The key pad is a 4 X 3 matrix, to test for a button being pressed we
must loop though, setting an output high(row), then testing the inputs (columns).
From this we can determine if a button is pressed or not.

This code uses an H-Bridge with a bi-stable solenoid. The solenoid changes state
by switching the polarity of the output to the solenoid via the H-Bridge.

H-Bridge SN754410 (pin out)
    1,2EN  |1  16|  Vcc1
       1A  |2  15|  4A
       1Y  |3  14|  4Y
      GRD  |4  13|  GRD
      GRD  |5  12|  GRD
       2Y  |6  11|  3Y
       2A  |7  10|  3A
     Vcc2  |8   9|  3,4E
     
     Using pins 1,2,7,8,16
     
     Vcc  5v - 16
     vcc 12v -  8
     
*/

#define KeyMask_0                   0x0001
#define KeyMask_1                   0x0002
#define KeyMask_2                   0x0004
#define KeyMask_3                   0x0008
#define KeyMask_4                   0x0010
#define KeyMask_5                   0x0020
#define KeyMask_6                   0x0040
#define KeyMask_7                   0x0080
#define KeyMask_8                   0x0100
#define KeyMask_9                   0x0200
#define KeyMask_CANCEL              0x0400
#define KeyMask_ENTER               0x0800
//key value integral values
#define KeyValue_0                  0
#define KeyValue_1                  1
#define KeyValue_2                  2
#define KeyValue_3                  3
#define KeyValue_4                  4
#define KeyValue_5                  5
#define KeyValue_6                  6
#define KeyValue_7                  7
#define KeyValue_8                  8
#define KeyValue_9                  9
#define KeyValue_CANCEL             10
#define KeyValue_ENTER              11
#define KeyValue_NONE               0xFFFF
//key board IO
#define RowsStart                   2
#define ColumnStart                 6
#define ColCount                    3
#define RowCount                    4
//board IO
#define OutputEnable                0           //1,2EN - Solenoid enable pin 1
#define Output1                     1           //1A    - Solenoid supply 1
#define Output2                     10          //2A    - Solenoid supply 2
#define OutputBuzzer                11
#define OutputOKLED                 12
#define OutputErrorLED              13

#define DebounceTimeOut             50
#define ErrorTime                   500
#define MaxEntryValues              12
#define EnterTime                   3000
#define Open                        0
#define Close                       1

void cancel();
void keyUp(unsigned short key);
void keyDown(unsigned short key);
void setSolenoid(unsigned short state);

unsigned short readKeys();
unsigned short getSelectedKey(unsigned short keys);

bool isValidAccessAttempt();
unsigned short currentAccessAttempt[MaxEntryValues] = { KeyValue_NONE };
unsigned short masterKeyCode[] = { KeyValue_4, KeyValue_2, KeyValue_1, KeyValue_6 };
unsigned short lastKeyValues = 0;
unsigned short currentEntryCodeIndex = 0;
unsigned short currentState = Close;
 
void setup() 
{
    // set the digital pin as output for rows
    for (int i = RowsStart; i < (RowsStart + RowCount); ++i) 
    {
        pinMode(i, OUTPUT); 
        digitalWrite(i, HIGH); 
    }
    
    // set the digital pin as inputs for columns, not really required as we default to input
    //but we do want to enable internal pullup resistors
    for (int i = ColumnStart; i < (ColumnStart + ColCount); ++i) 
    {
        pinMode(i, INPUT);          
        digitalWrite(i, HIGH);       // turn on pullup resistors 
    }     
    
    pinMode(OutputEnable,    OUTPUT);
    pinMode(Output1,         OUTPUT);
    pinMode(Output2,         OUTPUT);    
    pinMode(OutputOKLED,     OUTPUT); 
    pinMode(OutputErrorLED,  OUTPUT); 
    pinMode(OutputBuzzer,    OUTPUT); 
   
    setSolenoid(Close); 
}

void loop()
{
    unsigned short keys = readKeys();

    //we have a key state change
    if (keys != lastKeyValues)
    {
        //filter out the crap
        delay(DebounceTimeOut);
        keys = readKeys();
        if (keys == lastKeyValues)
            return;

        unsigned short oldKey = getSelectedKey(lastKeyValues);
        unsigned short keyValue = getSelectedKey(keys);
        lastKeyValues = keys;

        //if we are here we must have a key state change and have successfully filtered bouncing
        if (keyValue == KeyValue_CANCEL)
        {
            //reset the access attempt close if open
            cancel();
            setSolenoid(Close);
            return;
        } 
        
        if (keyValue == KeyValue_ENTER || currentState == Open)
        {
            //only process enter on the up stroke
            return;
        }        

        if (keyValue == KeyValue_NONE)
        {
            keyUp(oldKey);
            return;
        }

        //got here, so we must be entering a code for an access attempt
        if (currentEntryCodeIndex >= MaxEntryValues)
        {
            cancel();
            return;
        }

        currentAccessAttempt[currentEntryCodeIndex++] = keyValue;
        keyDown(keyValue);
    }
}

//Gets the intergal value of the currently pressed key 
unsigned short getSelectedKey(unsigned short keys)
{
    if (keys == 0x0000)
        return KeyValue_NONE;
    
    for (unsigned short i = 0; i < 16;++i)
    {
        unsigned short bitPlacement = 1 << i;
    
        if ((bitPlacement & keys) == bitPlacement)
            return i;
    }

    return KeyValue_NONE;
}

unsigned short readKeys()
{
    unsigned short keyStates = 0;
    //loop all the rows and set the row output
    //test each column result set the correct bit within the result
    for (short i = 0; i < RowCount;++i)
    {
        //set row output high
        digitalWrite(RowsStart + i, LOW);
        //test each column input
        for (short j = 0; j < ColCount; ++j)
        {
            int val = digitalRead(ColumnStart + j);
            //set a bit that corresponds to the value, see mappings
            unsigned short bitIndex = ((i * (RowCount-1)) + j);
            
            //this little bit of wizardry sorts out the values to what we want
            //the '0' key is on the same row as the 'CANCEL' and 'ENTER' keys
            //so we need to switch things round abit as '0' must become bit 0 not 10
            if (bitIndex == KeyValue_CANCEL)
                bitIndex = KeyValue_0;
            else if (bitIndex < KeyValue_CANCEL)
                bitIndex++;
            
            unsigned short bitPlacement = 1 << bitIndex;
      
            if (val == LOW)
            {                         
                //set the bit high, preserve all others
                keyStates |= bitPlacement;
            }
            else
            {
                unsigned short mask = 0xFFFF;
                //set bit to zero
                mask ^= bitPlacement;
                keyStates &= mask;
            }
        }
        //reset row output
        digitalWrite(RowsStart + i, HIGH);  
    }
  
    return keyStates;
}

void cancel()
{
    //reset the access attempt
    for (int i = 0;i < MaxEntryValues;++i)
        currentAccessAttempt[i] = KeyValue_NONE;

    currentEntryCodeIndex = 0;
    
    lastKeyValues = KeyValue_NONE;
}

bool isValidAccessAttempt()
{
    bool result = false;
    unsigned short masterCodeSize = (unsigned short)(sizeof(masterKeyCode) / sizeof(unsigned short));
 
    if ((currentEntryCodeIndex) == masterCodeSize)
    {
        for (int i = 0; i < masterCodeSize;++i)
        {
            if (masterKeyCode[i] != currentAccessAttempt[i])
            {
                result = false;
                break;
            }            
            result = true;
        }
    }

    cancel();
    return result;
}

void keyUp(unsigned short key)
{
    //stop buzzer
    if (key == KeyValue_ENTER)
    {
        //OK lets validate the entered keys against current code
        if (isValidAccessAttempt())
        {
            //cool, lets open that door!!!   
            digitalWrite(OutputBuzzer, HIGH);
            setSolenoid(Open);         
            delay(EnterTime);
            digitalWrite(OutputBuzzer, LOW);                                  
        }
        else
        {
            //oops... this is no good so set the red led output and buzzer
            digitalWrite(OutputErrorLED, HIGH);
            digitalWrite(OutputBuzzer,   HIGH);
            delay(ErrorTime);
            digitalWrite(OutputErrorLED, LOW); 
            digitalWrite(OutputBuzzer,   LOW); 
            delay(ErrorTime);
            digitalWrite(OutputErrorLED, HIGH);
            digitalWrite(OutputBuzzer,   HIGH);
            delay(ErrorTime); 
            digitalWrite(OutputBuzzer,   LOW);             
        }
    }
    else
        digitalWrite(OutputBuzzer, LOW);
}

void keyDown(unsigned short key)
{
    //start buzzer
    digitalWrite(OutputBuzzer, HIGH);
}

void setSolenoid(unsigned short state)
{
    if (state == Open)
    {
        digitalWrite(Output1,        LOW);
        digitalWrite(Output2,        HIGH); 
        digitalWrite(OutputOKLED,    HIGH);
        digitalWrite(OutputErrorLED, LOW);        
    }   
    else
    {
        digitalWrite(Output1,        HIGH);
        digitalWrite(Output2,        LOW);  
        digitalWrite(OutputOKLED,    LOW);   
        digitalWrite(OutputErrorLED, HIGH);     
    }
    
    currentState = state;
    
    digitalWrite(OutputEnable, HIGH);        
    delay(500);
    digitalWrite(OutputEnable, LOW);  
}



