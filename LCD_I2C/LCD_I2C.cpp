#include "Arduino.h"
#include <inttypes.h>
#include "Wire.h"
#include "LCD_I2C.h"

#if !defined(ARDUINO)
#error This library not designed for other-than-arduino use
#endif


// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set:
// DL = 1; 8-bit interface data
// N = 0; 1-line display
// F = 0; 5x8 dot character font
// 3. Display on/off control:
// D = 0; Display off
// C = 0; Cursor off
// B = 0; Blinking off
// 4. Entry mode set:
// I/D = 1; Increment by 1
// S = 0; No shift
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts(and the
// LiquidCrystal constructor is called) .

LCD_I2C::LCD_I2C(uint8_t lcd_Addr, uint8_t lcd_lines, uint8_t lcd_cols)
{
    _Addr  = lcd_Addr;
    _lines = lcd_lines;
    _cols  = lcd_cols;
    _backlightval = LCD_NOBACKLIGHT;
}

void LCD_I2C::init()
{
    Wire.begin();
    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40ms after power rises above 2.7V
    // before sending commands. Arduino can turn on way before 4.5V so we'll wait 50ms
    delay(50);
    // Now we pull both RS and R/W low to begin commands
    expanderWrite(_backlightval);	// reset expander and turn backlight off
    //put the LCD into 4 bit mode
    // this is according to the hitachi HD44780 datasheet fig 24, pg 46
    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03<<4);
    delayMicroseconds(4500); // wait min 4.1ms
    // second try
    write4bits(0x03<<4);
    delayMicroseconds(4500); // wait min 4.1ms
    // third go!
    write4bits(0x03<<4);
    delayMicroseconds(150);
    // finally, set to 4-bit interface
    write4bits(0x02<<4);
    // set # lines, font size, etc.
    _displayfunction = LCD_4BITMODE | LCD_5x8DOTS | ((_lines>1)?LCD_2LINE:LCD_1LINE);
    send(LCD_FUNCTIONSET | _displayfunction,0);
    _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    send(LCD_DISPLAYCONTROL | _displaycontrol,0);
    _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    send(LCD_ENTRYMODESET | _displaymode,0);
    clear();
}

/********** high level commands, for the user! */
void LCD_I2C::clear()
{
    send(LCD_CLEARDISPLAY,0);
    delayMicroseconds(2000);
}

void LCD_I2C::home()
{
    send(LCD_RETURNHOME,0);
    delayMicroseconds(2000);
}

void LCD_I2C::setPos(uint8_t line, uint8_t col)
{
    uint8_t offset=((line&1)?0x40:0)+((line&2)?0x14:0);
    send(LCD_SETDDRAMADDR|(col+offset),0);
}

void LCD_I2C::noDisplay()
{
    _displaycontrol &= ~LCD_DISPLAYON;
    send(LCD_DISPLAYCONTROL | _displaycontrol,0);
}

void LCD_I2C::display()
{
    _displaycontrol |= LCD_DISPLAYON;
    send(LCD_DISPLAYCONTROL | _displaycontrol,0);
}

void LCD_I2C::noCursor()
{
    _displaycontrol &= ~LCD_CURSORON;
    send(LCD_DISPLAYCONTROL | _displaycontrol,0);
}

void LCD_I2C::cursor()
{
    _displaycontrol |= LCD_CURSORON;
    send(LCD_DISPLAYCONTROL | _displaycontrol,0);
}

void LCD_I2C::noBlink()
{
    _displaycontrol &= ~LCD_BLINKON;
    send(LCD_DISPLAYCONTROL | _displaycontrol,0);
}

void LCD_I2C::blink()
{
    _displaycontrol |= LCD_BLINKON;
    send(LCD_DISPLAYCONTROL | _displaycontrol,0);
}

void LCD_I2C::scrollDisplayLeft()
{
    send(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT,0);
}

void LCD_I2C::scrollDisplayRight()
{
    send(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT,0);
}

// This is for text that flows Left to Right
void LCD_I2C::leftToRight()
{
    _displaymode |= LCD_ENTRYLEFT;
    send(LCD_ENTRYMODESET | _displaymode,0);
}

// This is for text that flows Right to Left
void LCD_I2C::rightToLeft()
{
    _displaymode &= ~LCD_ENTRYLEFT;
    send(LCD_ENTRYMODESET | _displaymode,0);
}

// This will 'right justify' text from the cursor
void LCD_I2C::autoscroll()
{
    _displaymode |= LCD_ENTRYSHIFTINCREMENT;
    send(LCD_ENTRYMODESET | _displaymode,0);
}

// This will 'left justify' text from the cursor
void LCD_I2C::noAutoscroll()
{
    _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
    send(LCD_ENTRYMODESET | _displaymode,0);
}

// Allows us to fill the first 8 CGRAM locations with custom characters
void LCD_I2C::createChar(uint8_t location, uint8_t charmap[])
{
    location &= 0x7;
    send(LCD_SETCGRAMADDR | (location << 3),0);
    for(int i=0; i<8; i++) {
        write(charmap[i]);
    }
}

void LCD_I2C::noBacklight()
{
    _backlightval=LCD_NOBACKLIGHT;
    expanderWrite(0);
}

void LCD_I2C::backlight()
{
    _backlightval=LCD_BACKLIGHT;
    expanderWrite(0);
}

/************ low level data pushing commands **********/
void LCD_I2C::send(uint8_t value, uint8_t mode)
{
    uint8_t highnib=value&0xf0;
    uint8_t lownib=(value<<4)&0xf0;
    write4bits((highnib)|mode);
    write4bits((lownib)|mode);
}

inline size_t LCD_I2C::write(uint8_t value) {
	send(value, Rs);
	return 1;
}

void LCD_I2C::write4bits(uint8_t value)
{
    expanderWrite(value);
    pulseEnable(value);
}
void LCD_I2C::expanderWrite(uint8_t _data)
{
    Wire.beginTransmission(_Addr);
    Wire.write((int)(_data) | _backlightval);
    Wire.endTransmission();
}
void LCD_I2C::pulseEnable(uint8_t _data)
{
    // En high
    expanderWrite(_data | En);
    // enable pulse must be >450ns
    delayMicroseconds(1);
    // En low
    expanderWrite(_data & ~En);
    // commands need > 37us to settle
    delayMicroseconds(50);
}

void LCD_I2C::printstr(const char c[])
{
    const char* p=c;
    while (*p) {
      write(*p);
      p++;
    }
}

// unsupported API functions
void LCD_I2C::draw_horizontal_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_col_end) { }
void LCD_I2C::draw_vertical_graph(uint8_t row, uint8_t column, uint8_t len, uint8_t pixel_row_end) { }
