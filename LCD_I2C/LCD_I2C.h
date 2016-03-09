#ifndef LCD_I2C_h
#define LCD_I2C_h

// This library has been stripped to minimize memory usage.
// Pretty much only usable with the particular hardware I have
// 2x16 LCD with I2C 8-bit interface expander backback.
// 4BIT communication ONLY, no 8 bit mode has been removed.
// lornix 2016-03-09

#if !defined (ARDUINO)
#error This library not designed for other-than-arduino use
#endif

#include <inttypes.h>
#include "Print.h"

// commands
#define LCD_CLEARDISPLAY   0x01
#define LCD_RETURNHOME     0x02
#define LCD_ENTRYMODESET   0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT    0x10
#define LCD_FUNCTIONSET    0x20
#define LCD_SETCGRAMADDR   0x40
#define LCD_SETDDRAMADDR   0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT          0x00
#define LCD_ENTRYLEFT           0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON  0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON   0x02
#define LCD_CURSOROFF  0x00
#define LCD_BLINKON    0x01
#define LCD_BLINKOFF   0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE  0x00
#define LCD_MOVERIGHT   0x04
#define LCD_MOVELEFT    0x00

// flags for function set
#define LCD_4BITMODE 0x00
#define LCD_2LINE    0x08
#define LCD_1LINE    0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS  0x00

// flags for backlight control
#define LCD_BACKLIGHT   0x08
#define LCD_NOBACKLIGHT 0x00

#define En B00000100  // Enable bit
#define Rw B00000010  // Read/Write bit
#define Rs B00000001  // Register select bit

class LCD_I2C : public Print {
 public:
     LCD_I2C(uint8_t lcd_Addr,uint8_t lcd_lines, uint8_t lcd_cols);
     void clear();
     void home();
     void backlight(uint8_t state=true);
     void blink(uint8_t state=true);
     void cursor(uint8_t state=true);
     void display(uint8_t state=true);
     void scrollDisplayLeft();
     void scrollDisplayRight();
     void printLeft();
     void printRight();
     void leftToRight();
     void rightToLeft();
     void shiftIncrement();
     void shiftDecrement();
     void autoscroll();
     void noAutoscroll();
     void createChar(uint8_t, uint8_t[]);
     void setPos(uint8_t, uint8_t);
     virtual size_t write(uint8_t);
     void init();

     ////compatibility API function aliases
     void load_custom_character(uint8_t char_num, uint8_t *rows);   // alias for createChar()
     void printstr(const char[]);

 private:
     void send(uint8_t, uint8_t);
     void write4bits(uint8_t);
     void expanderWrite(uint8_t);
     uint8_t _Addr;
     uint8_t _displayfunction;
     uint8_t _displaycontrol;
     uint8_t _displaymode;
     uint8_t _cols;
     uint8_t _lines;
     uint8_t _backlightval;
};

#endif
