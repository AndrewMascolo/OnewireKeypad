#ifndef BitBool_h
  #define BitBool_h
  #include "Arduino.h"

  template< size_t _Count >
    class BitBool{
      protected:
        struct BitRef{
        
          BitRef( uint8_t &u_DataRef, const uint8_t u_Idx ) : u_Data( u_DataRef ), u_Index( 0x1 << u_Idx ) { return; }

          operator const bool() const { return this->u_Data & this->u_Index; }
          
          const bool operator =( const BitRef &b_Copy ) const { return *this = ( const bool ) b_Copy; }

          const bool operator =( const bool &b_Copy ) const
            {
              if( b_Copy )  this->u_Data |= this->u_Index;
              else          this->u_Data &= ~this->u_Index;
              return b_Copy;
            }      
            
          uint8_t &u_Data;
          uint8_t const u_Index;
        };  
      public:
        enum{ BitCount = _Count, ByteCount = ( BitCount / 0x8 ) + ( ( BitCount % 0x8 ) ? 0x1 : 0x0 ) };

        BitRef operator []( const uint16_t i_Index ) { return BitRef( this->u_Data[ i_Index >> 0x3 ], i_Index & 0x7 ); }  
        uint8_t u_Data[ ByteCount ];
  };
#endif