#include <OnewireKeypad.h>
char *_Data = "123A456B789C*0#D";
char KEYS[] = {
  '1', '2', '3', //'A',
  '4', '5', '6', //'B',
  '7', '8', '9', //'C',
  '*', '0', '#', //'D'
};

OnewireKeypad <Print, 12 > Keypad(Serial, KEYS, 4, 4, A0 );
long R[4] = {0, 1000, 2200, 3300};
long C[4] = {0, 220, 470, 680};
void setup ()
{
  Serial.begin(115200);
  Serial.println(_Data[1]);
  pinMode(13, OUTPUT);
  Keypad.SetResistors(R, C, 1500, true);
  Keypad.addEventKey(test, '#'); // Add Function to list | Key to look for
  Keypad.addEventKey(togglePin13, '*');
}

void loop()
{
  Keypad.ListenforEventKey(); // check if an EventKey is found
  Serial.println(Keypad.S_Getkey());
}

void test()
{
  Serial.println("This is a test");
  Keypad.deleteEventKey('#'); // remove function from list
}

void togglePin13()
{
  digitalWrite(13, !digitalRead(13));
}
