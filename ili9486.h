#ifndef ILI9486_H
#define ILI9486_H

#include "stm32f4xx_hal.h"
// LCD dimensions
#define LCD_W 320
#define LCD_H 480

#define BLACK       0x0000
#define NAVY        0x000F
#define DARKGREEN   0x03E0
#define DARKCYAN    0x03EF
#define MAROON      0x7800
#define PURPLE      0x780F
#define OLIVE       0x7BE0
#define LIGHTGREY   0xC618
#define GRAY        0x5AEB
#define DARKGREY    0x7BEF
#define BLUE        0x001F
#define GREEN       0x07E0
#define CYAN        0x07FF
#define RED         0xF800
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0
#define WHITE       0xFFFF
#define ORANGE      0xFD20
#define GREENYELLOW 0xAFE5
#define PINK        0xF81F

// LCD device structure
typedef struct {
    uint16_t width;     // Display width in pixels
    uint16_t height;    // Display height in pixels
    uint16_t setxcmd;   // Command to set X coordinate
    uint16_t setycmd;   // Command to set Y coordinate
    uint16_t wramcmd;   // Command to write to GRAM
    uint16_t rramcmd;   // Command to read from GRAM
} _lcd_dev;

// Global variables
extern _lcd_dev lcddev;
extern uint16_t POINT_COLOR;
extern uint16_t BACK_COLOR;
extern uint16_t DeviceCode;

// Function declarations
void LCD_GPIOInit(void);
//void LCD_write(uint16_t val);
uint16_t LCD_read(void);
//void LCD_WR_REG(uint16_t data);
//void LCD_WR_DATA(uint16_t data);
uint16_t LCD_RD_DATA(void);
//void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
void LCD_ReadReg(uint16_t LCD_Reg, uint8_t *Rval, int n);
void LCD_WriteRAM_Prepare(void);
void LCD_ReadRAM_Prepare(void);
//void Lcd_WriteData_16Bit(uint16_t Data);
uint16_t Lcd_ReadData_16Bit(void);
void LCD_RESET(void);
void LCD_Init(void);
void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd);
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
void LCD_DrawPoint(uint16_t x, uint16_t y);
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y);
void LCD_Clear(uint16_t Color);
void LCD_FillScreen(uint16_t color);
void LCD_WriteDataBuffer(uint8_t *data, uint32_t length);
void LCD_direction(uint8_t direction);
uint16_t LCD_Read_ID(void);
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void LCD_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void delay_us(uint32_t us);
void delay_ms(uint32_t ms);

//////
void write16Color(uint16_t color, int32_t count);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
/* EXTRA*/
void LCD_SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
void LCD_DrawPixel1(uint16_t x, uint16_t y, uint16_t color);
void LCD_DrawLine1(uint16_t x0, uint16_t y0, uint16_t x1, int16_t y1, uint16_t color);
/////// CUSTOM

void customLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);


#endif // ILI9486_H
