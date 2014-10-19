//OnewireKP.h

#ifndef OnewireKeypad_h
#define OnewireKeypad_h
#include <Arduino.h>
#include "BitBool.h"

#define NO_KEY '\0'
#define WAITING 0
#define PRESSED 1
#define RELEASED 2
#define HELD 3

struct MissingType{};

#ifdef LiquidCrystal_I2C_h 
  typedef LiquidCrystal_I2C LCDTYPE;
#else
  typedef MissingType LCDTYPE;
#endif

/*--- Main Class ---*/
template< typename T, unsigned MAX_KEYS >
class OnewireKeypad{
  public: 
	OnewireKeypad( T &port, char KP[], uint8_t Rows, uint8_t Cols, uint8_t Pin, int R1, int R2 ) 
		: port_( port ), latchedKey( BitBool< MAX_KEYS >() ), _Data( KP ), _Rows( Rows ), _Cols( Cols ), _Pin( Pin ),
		  holdTime( 500 ), startTime( 0 ), lastState( 0 ), lastRead( 0 ), debounceTime( 10 ), Num( 0 ), R1( R1 ), R2( R2 ) { }

	char	Getkey();
	void	SetHoldTime(unsigned long setH_Time)       { holdTime = setH_Time; }
	void	SetDebounceTime(unsigned long setD_Time)       { debounceTime = setD_Time; }
	uint8_t	Key_State();
	bool	Readpin()      { return analogRead(_Pin)? 1 : 0; }
	void	LatchKey();
	bool	checkLatchedKey(char _key);
	void	addEventKey(void (*userFunc)(void), char KEY);
	void	deleteEventKey(char KEY);
	void	ListenforEventKey();

  protected: 
	T &port_;
	
  private:
	BitBool< MAX_KEYS > latchedKey;    
	char *_Data;                      
	uint8_t _Rows, _Cols, _Pin;
	enum{ SIZE = MAX_KEYS };
    int Num;	
	unsigned long time;
	unsigned long holdTime;
	unsigned long startTime;
	unsigned long debounceTime;  
    bool state, lastState, lastRead;
	int R1, R2;    
};

struct{
    void(*intFunc)(void); 
	char keyHolder;
} Event[20];

template < typename T, typename U > struct IsSameType{ 
enum { Value = false }; 
};

template < typename T > struct IsSameType< T, T > { 
enum { Value = true }; 
};

template < typename T, unsigned MAX_KEYS >
char OnewireKeypad< T, MAX_KEYS >::Getkey()
{  
   if( int Reading = analogRead(_Pin))
   {
	if(millis() - startTime > debounceTime)
	{
	  for( byte i = 0, R = _Rows-1, C = _Cols-1; i < SIZE; i++ )
	  {
		float V = (5.0f * float( R2 )) / (float(R2) + (float(R1) * float(R)) + (float(R2) * float(C)));
		float Vfinal = V * (1023.0f / 5.0f);
		
		if(Reading <= (int(Vfinal) + 1.9f )) 
		  return _Data[(SIZE-1)-i];
		
		if( C == 0 )
		{
		  R--;
		  C = _Cols-1;
		}
        else C--;		
	  }
	  startTime = millis();
	}
  } 
  return NO_KEY; 
}

template < typename T, unsigned MAX_KEYS >
uint8_t OnewireKeypad< T, MAX_KEYS >::Key_State()
{
  if((state = Readpin()) != lastState)
	return ( (lastState = state) ? PRESSED : RELEASED); //MOD
  
  else if(Readpin())
  {
	time = millis();

	while(Readpin())
	{
	  if((millis() - time) > holdTime) return HELD;
	}
	lastState = 0;
	return RELEASED;
  }
  return WAITING;
}


template < typename T, unsigned MAX_KEYS >
void OnewireKeypad<T, MAX_KEYS >::LatchKey()
{  
  char output[20];
  bool PRINT = false;
  char read = Getkey();
  if(read != lastRead)
  {
	if( read )
	{
	  for( int idx = 0; idx < SIZE; idx++ )
	  {
		if(read == _Data[idx] )
        {		
		   strcpy_P( output, ( latchedKey[idx] = !latchedKey[idx] ) ? PSTR( "Key X was Latched" ) : PSTR( "Key X was Unlatched" ) ); //MOD
		  PRINT = true;
		  output[ 4 ] = read; //MOD <- very clever       
		}
	  }
	}
	lastRead = read;
   if( PRINT ) 
   {
     if( IsSameType< T, LCDTYPE >::Value)
	    port_.print(output); // Lcd display
	 else
        port_.println(output); // Serial display
   }
  }
}

template < typename T, unsigned MAX_KEYS >
bool OnewireKeypad< T, MAX_KEYS >::checkLatchedKey(char _key)
{  
  for( int idx = 0; idx < SIZE; idx++ )
  {
	if( _key == _Data[idx] ) return latchedKey[idx];    
  }
  return false;
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::addEventKey(void (*userFunc)(void), char KEY)
{
  Event[Num].intFunc = userFunc;
  Event[Num].keyHolder = KEY;
  if(Num < MAX_KEYS) Num++;
  else 
  {
    if( IsSameType< T, LCDTYPE >::Value)
	   port_.print("Too Many EventKeys"); // Lcd display
	else
       port_.println("Too Many EventKeys"); // Serial display
  }
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::deleteEventKey(char KEY)
{
  for(int idx = 0; idx < Num; idx++)
  {
    if(KEY == Event[ idx ].keyHolder)
	{
	  Event[ idx ].intFunc = NULL;
	  Event[ idx ].keyHolder = '~'; // CHANGED from '\0' to '~', because '\0' was causing issues.
    }
  }	
}

template < typename T, unsigned MAX_KEYS >
void OnewireKeypad< T, MAX_KEYS >::ListenforEventKey()
{
  for(int idx = 0; idx < Num; idx++)
  {
	if(Getkey() == Event[ idx ].keyHolder)
	{
	  Event[ idx ].intFunc();
	}
  }
}

#endif