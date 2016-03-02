#include <Wire.h>
#include <LCD_I2C.h>

// set the LCD address to 0x3F for a 16 chars and 2 line display
LCD_I2C lcd(0x3F,2,16);

void setup()
{
  lcd.init();                      // initialize the lcd
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setPos(0,3);
  lcd.print("Hello, world!");
  lcd.setPos(1,2);
  lcd.print("Arduino LCD IIC 2016");
}


void loop()
{
}
