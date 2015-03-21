/*

  bmp190_Nokia5100.ino
  
  Description:

  Website: electronut.in
  
  References:
  
  - https://learn.sparkfun.com/tutorials/bmp180-barometric-pressure-sensor-hookup-
  
  - https://github.com/adafruit/Adafruit-GFX-Library
  
  - https://github.com/adafruit/Adafruit-PCD8544-Nokia-5110-LCD-library

*/

#include <Wire.h>
#include <SFE_BMP180.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>


// Hardware SPI (faster, but must use certain hardware pins):
// SCK is LCD serial clock (SCLK) - this is pin 13 on Arduino Uno
// MOSI is LCD DIN - this is pin 11 on an Arduino Uno
// pin 5 - Data/Command select (D/C)
// pin 4 - LCD chip select (CS)
// pin 3 - LCD reset (RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
// Note with hardware SPI MISO and SS pins aren't used but will still be read
// and written to during SPI transfer.  Be careful sharing these pins!

SFE_BMP180 bmp180;

void setup()   {

  // for debugging
  Serial.begin(9600);

  // initialize display
  display.begin();
  
  // set contrast
  display.setContrast(50);

  // initialize BMP180
  bool success = bmp180.begin();
  
  if(success) {
    Serial.println("BMP180 init success");
  }
}

// animation
void showAnimation()
{
  // clear display
  display.clearDisplay();
  display.display();
  
  int W = display.width();
  int H = display.height();

  // draw a mountain
  display.drawLine(0, H-1, W/4, 0, BLACK);
  display.drawLine(W/4, 0, W/2, H/2, BLACK);
  display.drawLine(W/2, H/2, 3*W/4, H/4, BLACK);
  display.drawLine(3*W/4, H/4, W-1, H-1, BLACK);
  
  display.display();
    
  // draw pulsing marker
  display.drawCircle(3*W/8, H/4, 2, BLACK);
  display.fillCircle(3*W/8, H/4, 2, BLACK);
  display.display();
  delay(200);
  display.drawCircle(3*W/8, H/4, 4, BLACK);
  display.display();
  delay(200);
  display.drawCircle(3*W/8, H/4, 6, BLACK);
  display.display();
  delay(200);
  display.drawCircle(3*W/8, H/4, 8, BLACK);
  display.display();
  delay(200);
  
  delay(200);

  // clear display
  display.clearDisplay();
  display.display();
}

// main loop 
void loop() {

  showAnimation();  
  
  // take measurement  
  double T, P, A;
  bool success = measureBMP180(T, P, A);
  // don't update for unsuccessful measurement
  if (!success)
    return;
    
  // display 
  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);

  String strMsg;
  // temp
  char strT[16];
  dtostrf(T, 3, 2, strT);
  strMsg += "T = ";
  strMsg += strT;
  strMsg += " C\n";
  double TF = (T + 32)*5.0/9.0;
  dtostrf(TF, 3, 2, strT);
  strMsg += "    ";
  strMsg += strT;
  strMsg += " F\n";

  // altitude
  char strA[16];
  dtostrf(A, 5, 2, strA);
  strMsg += "A = ";
  strMsg += strA;
  strMsg += " m\n";
  double AF = 3.28*A;
  dtostrf(AF, 5, 2, strA);
  strMsg += "    ";
  strMsg += strA;
  strMsg += "ft\n";
  
  // pressure
  char strP[16];
  dtostrf(P, 5, 2, strP);
  strMsg += "P = ";
  strMsg += strP;
  strMsg += " hPA \n";
  dtostrf(0.75*P, 5, 2, strP);  
  display.println(strMsg);
  
  // show
  display.display();
  
  // wait
  delay(5000);
}

// get T, P, A readings from BMP180
bool measureBMP180(double& temp, double& pr, double& al)
{
  char status;
  double T,P,p0,a;
  bool success = false;
  
  // Start a temperature measurement:
  // If request is successful, the number of ms to wait is returned.
  // If request is unsuccessful, 0 is returned.
  status = bmp180.startTemperature();
  if (status != 0)
  {
    // Wait for the measurement to complete:
    delay(status);

    // Retrieve the completed temperature measurement:
    // Note that the measurement is stored in the variable T.
    // Function returns 1 if successful, 0 if failure.

    status = bmp180.getTemperature(T);
    if (status != 0)
    {
      // save temp
      temp = T;   
      
      // Start a pressure measurement:
      // The parameter is the oversampling setting, from 0 to 3 (highest res, longest wait).
      // If request is successful, the number of ms to wait is returned.
      // If request is unsuccessful, 0 is returned.

      status = bmp180.startPressure(3);
      if (status != 0)
      {
        // Wait for the measurement to complete:
        delay(status);

        // Retrieve the completed pressure measurement:
        // Note that the measurement is stored in the variable P.
        // Note also that the function requires the previous temperature measurement (T).
        // (If temperature is stable, you can do one temperature measurement for a number of pressure measurements.)
        // Function returns 1 if successful, 0 if failure.

        status = bmp180.getPressure(P,T);
        if (status != 0)
        {
          // save pressure
          pr = P;
  
          // calculate altitude using pressure at sea level
          al = bmp180.altitude(P, 1013.25);
          
          // set flag
          success = true;
        }
      }
    }
  }
  
  return success;
}

