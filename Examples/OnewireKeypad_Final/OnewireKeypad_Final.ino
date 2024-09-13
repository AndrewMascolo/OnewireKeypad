#include <OnewireKeypad.h>

char KEYS[] = {
  '1', '2', '3', 'A',
  '4', '5', '6', 'B',
  '7', '8', '9', 'C',
  '*', '0', '#', 'D'
};

OnewireKeypad <Print, 16 > myKeypad(Serial, KEYS, 4, 4, A0, 4700, 1000 );

void setup () {
  Serial.begin(115200);
  pinMode(13, OUTPUT);
  myKeypad.setDebounceTime(50);
  myKeypad.showRange();
}

void loop() {

  if ( char key = myKeypad.getkey() ) {
    Serial.println(key);
    
    digitalWrite(13, key == 'C'); // If key pressed is C, turn on LED, anything else will turn it off.
    
    switch (myKeypad.keyState()) {
      case PRESSED:
        Serial.println("PRESSED");
        Serial.println(analogRead(4));
        break;

      case RELEASED:
        Serial.println("RELEASED");
        break;

      case HELD:
        Serial.println("HOLDING");
        break;
    }
  }
}
