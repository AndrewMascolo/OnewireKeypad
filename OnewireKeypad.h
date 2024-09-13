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
#define KP_TOLERANCE 20														\

struct MissingType {};

#ifdef LiquidCrystal_I2C_h
typedef LiquidCrystal_I2C LCDTYPE;
#else
typedef MissingType LCDTYPE;
#endif

#if defined(ESP32)
#define A_D 4095
#define SUPPLY_VOLTAGE 3.3
#else
#define A_D 1023
#define SUPPLY_VOLTAGE 5.0
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
      : port_( port ), latchedKey( BitBool< MAX_KEYS >() ), _Data( KP ), _Rows( Rows ), _Cols( Cols ), _Pin( Pin ), holdTime( 500 ), debounceTime(200), startTime( 0 ), lastState( 0 ), lastRead( 0 ), voltage( SUPPLY_VOLTAGE ), pinRange(A_D), ANALOG_FACTOR(A_D / SUPPLY_VOLTAGE), index( 0 ), R1( R1 ), R2( R2 ), R3( R3 ) { }

    OnewireKeypad( T &port, char KP[], uint8_t Rows, uint8_t Cols, uint8_t Pin, int R1, int R2)
      : port_( port ), latchedKey( BitBool< MAX_KEYS >() ), _Data( KP ), _Rows( Rows ), _Cols( Cols ), _Pin( Pin ), holdTime( 500 ),debounceTime(200), startTime( 0 ), lastState( 0 ), lastRead( 0 ), voltage( SUPPLY_VOLTAGE ), pinRange(A_D), ANALOG_FACTOR(A_D / SUPPLY_VOLTAGE), index( 0 ), R1( R1 ), R2( R2 ) { }

    OnewireKeypad( T &port, char KP[], uint8_t Rows, uint8_t Cols, uint8_t Pin)
      : port_( port ), latchedKey( BitBool< MAX_KEYS >() ), _Data( KP ), _Rows( Rows ), _Cols( Cols ), _Pin( Pin ), holdTime( 500 ),debounceTime(200), startTime( 0 ), lastState( 0 ), lastRead( 0 ), voltage( SUPPLY_VOLTAGE ), pinRange(A_D), ANALOG_FACTOR(A_D / SUPPLY_VOLTAGE), index( 0 ) { }

    char	getkey();
    void	setHoldTime(unsigned long setH_Time) { holdTime = setH_Time; }
    void	setDebounceTime(unsigned long setD_Time) { debounceTime = setD_Time; }
	void	setKeypadVoltage(float volts);
	void	setAnalogPinRange(float range);
	void	setResistorTolerance(float value) { resistorTolerance = value; }
    uint8_t	keyState();
    bool	readPin() { return analogRead(_Pin) > KP_TOLERANCE; }
    void	latchKey();
    bool	checkLatchedKey(char _key);
    void	addEventKey(void (*userFunc)(void), char key);
    void	deleteEventKey(char key);
    void	listenforEventKey();
    void	showRange();
	uint16_t 	getPinRange() { return pinRange; }
	uint8_t _Pin;
	float resistorTolerance = 0.05;

  protected:
    T &port_;
    float ANALOG_FACTOR;

  private:
    BitBool< MAX_KEYS > latchedKey;
    char 	*_Data;
    uint8_t _Rows, _Cols; //, _Pin;
    enum { SIZE = MAX_KEYS };
    uint8_t index;
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
	void errorMSG(const char* msg);
};

template < typename T, typename U > struct IsSameType {
	enum { Value = false };
};

template < typename T > struct IsSameType< T, T > {
	enum { Value = true };
};

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad<T, MAX_KEYS>::errorMSG(const char* msg) {
	if ( IsSameType< T, LCDTYPE >::Value) {
		port_.print(msg); // Lcd display
	} else {
		port_.println(msg); // Serial display
	}
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::setAnalogPinRange(float range) {
	if (range <= 0 || range > A_D ) {
		char charArray[80];
		sprintf(charArray, "Error. pinRange must not be less than or equal to 0 or greater than %d\0", A_D);
		errorMSG(charArray);
	} else {
		pinRange = range;
	}
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::setKeypadVoltage(float volts) {
	if (pinRange == 0)
		errorMSG("Error, PinRange is 0.\nIf you are setting the keypad voltage, then you must also set the PinRange");
	
	if (volts <= 0 || volts > SUPPLY_VOLTAGE) {
		char charArray[80];
		sprintf(charArray, "Error. pinRange must not be less than or equal to 0 or greater than %f\0", SUPPLY_VOLTAGE);
		errorMSG(charArray);
	} else {
		voltage = volts;
		ANALOG_FACTOR = (pinRange / volts);
	}
}

template < typename T, unsigned MAX_KEYS >
char OnewireKeypad< T, MAX_KEYS >::getkey() {
	// Check R3 and set it if needed
	if ( R3 == 0 ) { R3 = R2; }

	boolean state = readPin();
	
	int pinReading = analogRead(_Pin);
		
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
			
			int highExtreme = Vfinal * (1 + resistorTolerance);
			int lowExtreme = Vfinal * (1 - resistorTolerance);
			
			if ( pinReading <= highExtreme && pinReading >= lowExtreme) {
				return _Data[(SIZE - 1) - i];
			}

			if ( C == 0 ) {
				R--;
				C = _Cols - 1;
			} else { C--; }
		}
	}

  return NO_KEY;
}

template < typename T, unsigned MAX_KEYS >
uint8_t OnewireKeypad< T, MAX_KEYS >::keyState() {
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
void OnewireKeypad<T, MAX_KEYS >::latchKey() {
	char output[20];
	bool PRINT = false;
	char read = getkey();
	
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
			errorMSG(output);
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
void OnewireKeypad< T, MAX_KEYS >::addEventKey(void (*userFunc)(void), char key) {
	Event[index].intFunc = userFunc;
	Event[index].keyHolder = key;
	if (index < MAX_KEYS) {
		index++;
	} else {
		errorMSG("Too Many EventKeys");
	}
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::deleteEventKey(char key) {
	for (uint8_t idx = 0; idx < index; idx++) {
		if (key == Event[ idx ].keyHolder) {
			Event[ idx ].intFunc = NULL;
			Event[ idx ].keyHolder = '~'; // CHANGED from '\0' to '~', because '\0' was causing issues.
			break;
		}
	}
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::listenforEventKey() {
	for (uint8_t idx = 0; idx < index; idx++) {
		if (getkey() == Event[ idx ].keyHolder) {
			if (keyState() == RELEASED) {
				Event[ idx ].intFunc();
				break;
			}
		}
	}
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::showRange() {
	if (R3 == 0) { R3 = R2; }

	for ( uint8_t R = 0; R < _Rows; R++) {
		for ( uint8_t C = 0; C < _Cols; C++) {
			float V = (voltage * float( R3 )) / (float(R3) + (float(R1) * float(R)) + (float(R2) * float(C)));
      
			if ( !IsSameType< T, LCDTYPE >::Value)
				port_ << "V:" << V << ", AR: " << (V * ANALOG_FACTOR) << " | "; // 204.6 is from A_D/5.0
		}
		if ( !IsSameType< T, LCDTYPE >::Value)
			port_.println("\n--------------------------------------------------------------------------------");
	}
}

#endif
