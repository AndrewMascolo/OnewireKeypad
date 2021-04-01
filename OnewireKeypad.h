//OnewireKP.h
/*
The MIT License (MIT)

Copyright (c) 2016 Andrew Mascolo Jr

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef OnewireKeypad_h
#define OnewireKeypad_h
#include <Arduino.h>
#include "BitBool.h"

#define NO_KEY '\0'
#define WAITING 0
#define PRESSED 1
#define RELEASED 2
#define HELD 3

//This number is based on 1/3 the lowest AR value from the ShowRange function.
#define KP_TOLERANCE 20

struct MissingType {};

#ifdef LiquidCrystal_I2C_h
typedef LiquidCrystal_I2C LCDTYPE;
#else
typedef MissingType LCDTYPE;
#endif

template<class T> inline Print &operator<<(Print &Outport, T str) {
	Outport.print(str);
	return Outport;
}

/*--- Main Class ---*/
template< typename T, unsigned MAX_KEYS >
class OnewireKeypad {
  public:
    OnewireKeypad( T &port, char KP[], uint8_t Rows, uint8_t Cols, uint8_t Pin, long R1, int R2, int R3)
      : port_( port ), latchedKey( BitBool< MAX_KEYS >() ), _Data( KP ), _Rows( Rows ), _Cols( Cols ), _Pin( Pin ), holdTime( 500 ), debounceTime(200), startTime( 0 ), lastState( 0 ), lastRead( 0 ), voltage( 5.0 ), ANALOG_FACTOR(1023 / 5.0), Num( 0 ), R1( R1 ), R2( R2 ), R3( R3 ) { }

    OnewireKeypad( T &port, char KP[], uint8_t Rows, uint8_t Cols, uint8_t Pin, int R1, int R2)
      : port_( port ), latchedKey( BitBool< MAX_KEYS >() ), _Data( KP ), _Rows( Rows ), _Cols( Cols ), _Pin( Pin ), holdTime( 500 ),debounceTime(200), startTime( 0 ), lastState( 0 ), lastRead( 0 ), voltage( 5.0 ), ANALOG_FACTOR(1023 / 5.0), Num( 0 ), R1( R1 ), R2( R2 ) { }

    OnewireKeypad( T &port, char KP[], uint8_t Rows, uint8_t Cols, uint8_t Pin)
      : port_( port ), latchedKey( BitBool< MAX_KEYS >() ), _Data( KP ), _Rows( Rows ), _Cols( Cols ), _Pin( Pin ), holdTime( 500 ),debounceTime(200), startTime( 0 ), lastState( 0 ), lastRead( 0 ), voltage( 5.0 ), ANALOG_FACTOR(1023 / 5.0), Num( 0 ) { }

    char	Getkey();
    char	S_Getkey();
    void	SetHoldTime(unsigned long setH_Time) { holdTime = setH_Time; }
    void	SetDebounceTime(unsigned long setD_Time) { debounceTime = setD_Time; }
	void	SetKeypadVoltage(float Volts);
	void	SetAnalogPinRange(float range);
    uint8_t	Key_State();
    bool	readPin() { return analogRead(_Pin) > KP_TOLERANCE; }
    void	LatchKey();
    bool	checkLatchedKey(char _key);
    void	addEventKey(void (*userFunc)(void), char KEY);
    void	deleteEventKey(char KEY);
    void	ListenforEventKey();
    void	ShowRange();
	uint8_t _Pin;

  protected:
    T &port_;
    float ANALOG_FACTOR;

  private:
    BitBool< MAX_KEYS > latchedKey;
    char 	*_Data;
    uint8_t _Rows, _Cols; //, _Pin;
    enum { SIZE = MAX_KEYS };
    uint8_t Num;
    float 	voltage;
    unsigned long time;
    unsigned long holdTime;
    unsigned long startTime;
    bool 	state, lastState, lastRead;
    long 	R1, R2, R3;
    unsigned long debounceTime, lastDebounceTime;
    unsigned int pinRange;
	float 	lastReading;
    struct {
		void(*intFunc)();
		char keyHolder;
    } Event[MAX_KEYS];
};

template < typename T, typename U > struct IsSameType {
	enum { Value = false };
};

template < typename T > struct IsSameType< T, T > {
	enum { Value = true };
};

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::SetAnalogPinRange(float range) {
	if (range <= 0 || range > 1023 ) {
		if ( IsSameType< T, LCDTYPE >::Value) {
			port_.print("Error. pinRange must not be less than or equal to 0 or greater than 1023"); // Lcd display
		} else {
			port_.println("Error. pinRange must not be less than or equal to 0 or greater than 1023"); // Serial display
		}
	} else {
		pinRange = range;
	}
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::SetKeypadVoltage(float Volts) {
	if (Volts <= 0 || Volts > 5) {
		if ( IsSameType< T, LCDTYPE >::Value) {
			port_.print("Error. The Voltage must not be less than or equal to 0 or greater than 5"); // Lcd display
		} else {
			port_.println("Error. The Voltage must not be less than or equal to 0 or greater than 5"); // Serial display
		}
	} else {
		voltage = Volts;
		ANALOG_FACTOR = (pinRange / Volts);
	}
}

template < typename T, unsigned MAX_KEYS >
char OnewireKeypad< T, MAX_KEYS >::Getkey() {
	// Check R3 and set it if needed
	if ( R3 == 0 ) { R3 = R2; }

	boolean state = readPin();
	
	int pinReading = analogRead(_Pin);
	int currdiff = 0;
	int lastdiff = 1025;

	unsigned long currentMillis = millis();
		
	if (state != lastReading ) {
		lastDebounceTime = currentMillis;
		lastReading = state;
	}
		
	if ( ((currentMillis - lastDebounceTime) > debounceTime) ) {
		if (state == false) { return NO_KEY; }
			
		for ( uint8_t i = 0, R = _Rows - 1, C = _Cols - 1; i < SIZE; i++ ) {
			float V = (voltage * float( R3 )) / (float(R3) + (float(R1) * float(R)) + (float(R2) * float(C)));
			float Vfinal = V * ANALOG_FACTOR;
			
			// Find closest key value to input reading 
			currdiff = int(Vfinal) - pinReading;
			currdiff = (currdiff < 0) ? (currdiff * -1) : currdiff; // abs(diff)

			if ( currdiff == 0) {
				return _Data[(SIZE - 1) - i];
			} else if ( currdiff >= lastdiff) { 		// Return last checked value
				return _Data[(SIZE - 1) - i + 1 ];
			} else if ( i == SIZE - 1) { 				// Reached end of list
				return _Data[(SIZE - 1) - i];
			} else {
				lastdiff = currdiff;
			}

			if ( C == 0 ) {
				R--;
				C = _Cols - 1;
			} else { C--;}
		}
	}

  return NO_KEY;
}

template < typename T, unsigned MAX_KEYS >
uint8_t OnewireKeypad< T, MAX_KEYS >::Key_State() {
	if ((state = readPin()) != lastState) {
		return ( (lastState = state) ? PRESSED : RELEASED); //MOD
	} else if (state) {
		time = millis();

		while (readPin()) {
			if ((millis() - time) > holdTime) { return HELD; }
		}
		lastState = 0;
		return RELEASED;
	}
	return WAITING;
}


template < typename T, unsigned MAX_KEYS >
void OnewireKeypad<T, MAX_KEYS >::LatchKey() {
	char output[20];
	bool PRINT = false;
	char read = Getkey();
	
	if (read != lastRead) {
		if ( read ) {
			for ( int idx = 0; idx < SIZE; idx++ ) {
				if (read == _Data[idx] ) {
					strcpy_P( output, ( latchedKey[idx] = !latchedKey[idx] ) ? PSTR( "Key X was Latched" ) : PSTR( "Key X was Unlatched" ) ); //MOD
					PRINT = true;
					output[ 4 ] = read; //MOD <- very clever
				}
			}
		}
    
		lastRead = read;
		if ( PRINT ) {
			if ( IsSameType< T, LCDTYPE >::Value) {
				port_.print(output); // Lcd display
			} else {
				port_.println(output); // Serial display
			}
		}
	}
}

template < typename T, unsigned MAX_KEYS >
bool OnewireKeypad< T, MAX_KEYS >::checkLatchedKey(char _key) {
	for ( uint8_t idx = 0; idx < SIZE; idx++ ) {
		if ( _key == _Data[idx] ) { return latchedKey[idx]; }
	}
	return false;
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::addEventKey(void (*userFunc)(void), char KEY) {
	Event[Num].intFunc = userFunc;
	Event[Num].keyHolder = KEY;
	if (Num < MAX_KEYS) {
		Num++;
	} else {
		if ( IsSameType< T, LCDTYPE >::Value) {
			port_.print("Too Many EventKeys"); // Lcd display
		} else {
			port_.println("Too Many EventKeys"); // Serial display
		}
	}
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::deleteEventKey(char KEY) {
	for (uint8_t idx = 0; idx < Num; idx++) {
		if (KEY == Event[ idx ].keyHolder) {
			Event[ idx ].intFunc = NULL;
			Event[ idx ].keyHolder = '~'; // CHANGED from '\0' to '~', because '\0' was causing issues.
			break;
		}
	}
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::ListenforEventKey() {
	for (uint8_t idx = 0; idx < Num; idx++) {
		if (Getkey() == Event[ idx ].keyHolder) {
			if (Key_State() == RELEASED) {
				Event[ idx ].intFunc();
				break;
			}
		}
	}
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::ShowRange() {
	if (R3 == 0) { R3 = R2; }

	for ( uint8_t R = 0; R < _Rows; R++) {
		for ( uint8_t C = 0; C < _Cols; C++) {
			float V = (voltage * float( R3 )) / (float(R3) + (float(R1) * float(R)) + (float(R2) * float(C)));
      
			if ( !IsSameType< T, LCDTYPE >::Value)
				port_ << "V:" << V << ", AR: " << (V * ANALOG_FACTOR) << " | "; // 204.6 is from 1023/5.0
		}
		if ( !IsSameType< T, LCDTYPE >::Value)
			port_.println("\n--------------------------------------------------------------------------------");
	}
}

#endif
