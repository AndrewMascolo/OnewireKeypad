//Original Creator: Andrew Mascolo

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <OnewireKeypad.h>

char KEYS[]= {
  '1','2','3',//'A',
  '4','5','6',//'B',
  '7','8','9',//'C',
  '*','0','#',//'D'
};
LiquidCrystal_I2C Lcd(0x20,20,4); 
OnewireKeypad <LiquidCrystal_I2C, 12> KP( Lcd, KEYS, 4, 3, A0, 4700, 1000 );
OnewireKeypad <Print, 12 > KP2(Serial, KEYS, 4, 3, A0, 4700, 1000 );

void setup (){
  Serial.begin(115200);
  Lcd.init();                      
  Lcd.backlight();
}
void loop() 
{
  if( KP.Getkey() )
  {
    Lcd.clear();
    Lcd.home();
    Lcd.print( F( "KP: " ) );
    Lcd.setCursor(4,0);
    KP.LatchKey();
  }

  if( KP2.Getkey() )
  {
    Serial.print( F( "KP2: " ) );
    Serial.println(KP2.Key_State());
    KP2.LatchKey();
    Serial.println();
  } 
}

