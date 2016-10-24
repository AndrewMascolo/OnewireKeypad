void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  showValues(4,4,4700,1000, 5);
}

void loop() {
  // put your main code here, to run repeatedly:

}

void showValues(int rows, int cols, long Rrows, long Rcols, int Volt)
{
   for( int R = 0; R < rows; R++)
   {
     for( int C = cols - 1; C >= 0; C--)
     {
        float V = (5.0f * float( Rcols )) / (float(Rcols) + (float(Rrows) * R) + (float(Rcols) * C));
        Serial.print(V); Serial.print(F("\t"));
     } 
     Serial.println();  
   }
}
