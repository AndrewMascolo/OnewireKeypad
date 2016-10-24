#include <OnewireKeypad.h>

char KEYS[]= {
  '1','2','3',
  '4','5','6',
  '7','8','9',
  '*','0','#',
};

OnewireKeypad <Print, 12 > Keypad(Serial, KEYS, 4, 3, A0, 4700, 1000, ExtremePrec );

void setup ()
{
  Serial.begin(115200);
  pinMode(13,OUTPUT);
  Keypad.addEventKey(test, '#'); // Add Function to list | Key to look for
  Keypad.addEventKey(togglePin13, '*');
}

void loop() 
{
  Keypad.ListenforEventKey(); // check if an EventKey is found
}

void test()
{
  Serial.println("This is a test");
  Keypad.deleteEventKey('#'); // remove function from list
}

void togglePin13()
{
  digitalWrite(13,!digitalRead(13));
}
