
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h>                            // I2C control library
#include <LiquidCrystal_I2C.h>          // LCD library

LiquidCrystal_I2C lcd(0x3F, 16, 2);  // set the LCD address to 0x20 for a 16 chars and 2 line display

void setup()
{
  lcd.init();                      // initialize the lcd 
  lcd.clear();
  // Print a message to the LCD.
  lcd.backlight();  // turn on backlight
  lcd.setCursor(0,0);
  lcd.print("hi");
  lcd.setCursor(0,1);
  lcd.print("Hello, world!");
}

void loop()
{
}
