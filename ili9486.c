#include "stm32f4xx_hal.h"
#include <stdlib.h>
#include <stdint.h>
#include "ili9486.h"
#include "Fonts/fonts.h"

// GPIO pin definitions
#define RST_Pin GPIO_PIN_8
#define RST_GPIO_Port GPIOA
#define CS_Pin GPIO_PIN_9
#define CS_GPIO_Port GPIOA
#define RS_Pin GPIO_PIN_10
#define RS_GPIO_Port GPIOA
#define WR_Pin GPIO_PIN_11
#define WR_GPIO_Port GPIOA
#define RD_Pin GPIO_PIN_12
#define RD_GPIO_Port GPIOA
#define DATA_Port GPIOD
#define DATA_Pins_Mask 0x00FF // PD0–PD7 mask (lower byte)

// Control macros
#define _swap_int16_t(a, b) do { int16_t t = a; a = b; b = t; } while (0)

// Reset pin PA8
#define LCD_RST_CLR() (GPIOA->BSRR = (uint32_t)(RST_Pin) << 16)
#define LCD_RST_SET() (GPIOA->BSRR = RST_Pin)

// Chip select PA9
#define LCD_CS_CLR()  (GPIOA->BSRR = (uint32_t)(CS_Pin) << 16)
#define LCD_CS_SET()  (GPIOA->BSRR = CS_Pin)

// Register select PA10
#define LCD_RS_CLR()  (GPIOA->BSRR = (uint32_t)(RS_Pin) << 16)
#define LCD_RS_SET()  (GPIOA->BSRR = RS_Pin)

// Write strobe PA11
#define LCD_WR_CLR()  (GPIOA->BSRR = (uint32_t)(WR_Pin) << 16)
#define LCD_WR_SET()  (GPIOA->BSRR = WR_Pin)

// Read strobe PA12
#define LCD_RD_CLR()  (GPIOA->BSRR = (uint32_t)(RD_Pin) << 16)
#define LCD_RD_SET()  (GPIOA->BSRR = RD_Pin)


/*
#define LCD_CS_CLR() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET)
#define LCD_CS_SET() HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET)
#define LCD_RS_CLR() HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_RESET)
#define LCD_RS_SET() HAL_GPIO_WritePin(RS_GPIO_Port, RS_Pin, GPIO_PIN_SET)
#define LCD_WR_CLR() HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_RESET)
#define LCD_WR_SET() HAL_GPIO_WritePin(WR_GPIO_Port, WR_Pin, GPIO_PIN_SET)
#define LCD_RD_CLR() HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_RESET)
#define LCD_RD_SET() HAL_GPIO_WritePin(RD_GPIO_Port, RD_Pin, GPIO_PIN_SET)
#define LCD_RST_CLR() HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET)
#define LCD_RST_SET() HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET)
*/
// Data macros
#define DATAOUT(data) LCD_WriteDataBus(data)
#define DATAIN() LCD_ReadDataBus()

// LCD dimensions
#define LCD_W 320
#define LCD_H 480

#define swap(a, b)			do {\
								int16_t t = a;\
								a = b;\
								b = t;\
							} while(0)
// LCD device structure

_lcd_dev lcddev = {
    .width = LCD_W,
    .height = LCD_H,
    .setxcmd = 0x2A,
    .setycmd = 0x2B,
    .wramcmd = 0x2C,
    .rramcmd = 0x2E
};



// Color definitions
uint16_t POINT_COLOR = 0x0000; // Black
uint16_t BACK_COLOR = 0xFFFF;  // White
uint16_t DeviceCode;

// Write 8-bit data to GPIO pins
static void LCD_WriteDataBus(uint8_t data) {
    DATA_Port->ODR = (DATA_Port->ODR & 0xFF00) | data;
}

// Read 8-bit data from GPIO pins
static inline uint8_t LCD_ReadDataBus(void) {
    return (uint8_t)(GPIOD->IDR & 0x00FF); // Read PD0–PD7
}

// GPIO initialization
void LCD_GPIOInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Enable GPIO clocks
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    // Configure control pins (CS, RS, RST, WR, RD)
    GPIO_InitStruct.Pin = CS_Pin | RS_Pin | RST_Pin | WR_Pin | RD_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // Configure data pins (PD0-PD7)
    GPIO_InitStruct.Pin = DATA_Pins_Mask;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    // Set initial states
    LCD_CS_SET();
    LCD_RS_SET();
    LCD_RST_SET();
    LCD_WR_SET();
    LCD_RD_SET();
}

// Low-level write
static inline void LCD_write(uint16_t val) {
    LCD_CS_CLR();
    DATAOUT((uint8_t)val);
    LCD_WR_CLR();
    LCD_WR_SET();
    LCD_CS_SET();
}

// Low-level read
uint16_t LCD_read(void) {
    uint16_t data;
    LCD_CS_CLR();
    LCD_RD_CLR();
    delay_us(1);
    data = DATAIN();
    LCD_RD_SET();
    LCD_CS_SET();
    return data;
}

// Write command
static inline void LCD_WR_REG(uint16_t data) {
    LCD_RS_CLR();
    LCD_write(data);
}

// Write data
static inline void LCD_WR_DATA(uint16_t data) {
    LCD_RS_SET();
    LCD_write(data);
}

// Read data
uint16_t LCD_RD_DATA(void) {
    LCD_RS_SET();
    return LCD_read();
}

// Write register
static inline void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue) {
    LCD_WR_REG(LCD_Reg);
    LCD_WR_DATA(LCD_RegValue);
}

// Read register
void LCD_ReadReg(uint16_t LCD_Reg, uint8_t *Rval, int n) {
    if (Rval == NULL || n <= 0) return;
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    LCD_WR_REG(LCD_Reg);
    GPIO_InitStruct.Pin = DATA_Pins_Mask;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    while (n--) {
        *(Rval++) = LCD_RD_DATA();
    }
    GPIO_InitStruct.Pin = DATA_Pins_Mask;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
}

// Prepare to write GRAM
void LCD_WriteRAM_Prepare(void) {
    LCD_WR_REG(lcddev.wramcmd);
}

// Prepare to read GRAM
void LCD_ReadRAM_Prepare(void) {
    LCD_WR_REG(lcddev.rramcmd);
}

// Write 16-bit data
static inline void Lcd_WriteData_16Bit(uint16_t Data) {
    LCD_RS_SET();
    LCD_CS_CLR();
    DATAOUT(Data >> 8); // High byte
    LCD_WR_CLR();
    LCD_WR_SET();
    DATAOUT(Data & 0xFF); // Low byte
    LCD_WR_CLR();
    LCD_WR_SET();
    LCD_CS_SET();
}




// Read 16-bit data
uint16_t Lcd_ReadData_16Bit(void) {
    uint16_t r, g, b;
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // Configure data pins as inputs
    GPIO_InitStruct.Pin = DATA_Pins_Mask;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    LCD_RS_SET();
    LCD_CS_CLR();

    // Dummy read
    LCD_RD_CLR();
    delay_us(1);
    LCD_RD_SET();

    // Read red
    LCD_RD_CLR();
    delay_us(1);
    r = DATAIN();
    LCD_RD_SET();

    // Read green
    LCD_RD_CLR();
    delay_us(1);
    g = DATAIN();
    LCD_RD_SET();

    // Read blue
    LCD_RD_CLR();
    delay_us(1);
    b = DATAIN();
    LCD_RD_SET();

    LCD_CS_SET();

    // Restore data pins as outputs
    GPIO_InitStruct.Pin = DATA_Pins_Mask;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    // Combine RGB (5-6-5 format)
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

// Reset LCD
void LCD_RESET(void) {
    LCD_RST_CLR();
    delay_ms(100);
    LCD_RST_SET();
    delay_ms(50);
}

// Initialize LCD (ILI9486)
void LCD_Init(void) {
    LCD_GPIOInit();
    LCD_RESET();

    // ILI9486 initialization sequence (from MCUFRIEND_kbv.cpp)
    LCD_WR_REG(0x01); // Soft Reset
    delay_ms(150);
    LCD_WR_REG(0x28); // Display Off
    LCD_WR_REG(0x3A); // Pixel Format
    LCD_WR_DATA(0x55); // 16-bit (565 format)

    // Power Control 1
    LCD_WR_REG(0xC0);
    LCD_WR_DATA(0x0D);
    LCD_WR_DATA(0x0D);

    // Power Control 2
    LCD_WR_REG(0xC1);
    LCD_WR_DATA(0x43);
    LCD_WR_DATA(0x00);

    // Power Control 3
    LCD_WR_REG(0xC2);
    LCD_WR_DATA(0x00);

    // VCOM Control
    LCD_WR_REG(0xC5);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x48);
    LCD_WR_DATA(0x00);
    LCD_WR_DATA(0x48);

    // Inversion Control
    LCD_WR_REG(0xB4);
    LCD_WR_DATA(0x00);

    // Display Function Control
    LCD_WR_REG(0xB6);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x02);
    LCD_WR_DATA(0x3B);

    // Gamma Settings (TM 3.2 Inch Initial Code)
    LCD_WR_REG(0xE0);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x21);
    LCD_WR_DATA(0x1C);
    LCD_WR_DATA(0x0B);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x08);
    LCD_WR_DATA(0x49);
    LCD_WR_DATA(0x98);
    LCD_WR_DATA(0x38);
    LCD_WR_DATA(0x09);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x03);
    LCD_WR_DATA(0x14);
    LCD_WR_DATA(0x10);
    LCD_WR_DATA(0x00);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA(0x0F);
    LCD_WR_DATA(0x2F);
    LCD_WR_DATA(0x2B);
    LCD_WR_DATA(0x0C);
    LCD_WR_DATA(0x0E);
    LCD_WR_DATA(0x06);
    LCD_WR_DATA(0x47);
    LCD_WR_DATA(0x76);
    LCD_WR_DATA(0x37);
    LCD_WR_DATA(0x07);
    LCD_WR_DATA(0x11);
    LCD_WR_DATA(0x04);
    LCD_WR_DATA(0x23);
    LCD_WR_DATA(0x1E);
    LCD_WR_DATA(0x00);

    // Sleep Out
    LCD_WR_REG(0x11);
    delay_ms(150);

    // Display On
    LCD_WR_REG(0x29);

    LCD_direction(1); // Set to portrait
    fillRect(0, 0, 479, 319, BLACK);
}

// Set display window
void LCD_SetWindows(uint16_t xStar, uint16_t yStar, uint16_t xEnd, uint16_t yEnd) {
    LCD_WR_REG(lcddev.setxcmd);
    LCD_WR_DATA(xStar >> 8);
    LCD_WR_DATA(0x00FF & xStar);
    LCD_WR_DATA(xEnd >> 8);
    LCD_WR_DATA(0x00FF & xEnd);

    LCD_WR_REG(lcddev.setycmd);
    LCD_WR_DATA(yStar >> 8);
    LCD_WR_DATA(0x00FF & yStar);
    LCD_WR_DATA(yEnd >> 8);
    LCD_WR_DATA(0x00FF & yEnd);

    LCD_WriteRAM_Prepare();
}

// Set cursor
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos) {
    LCD_SetWindows(Xpos, Ypos, Xpos, Ypos);
}

// Draw point
void LCD_DrawPoint(uint16_t x, uint16_t y) {
    if (x >= lcddev.width || y >= lcddev.height) return;
    LCD_SetCursor(x, y);
    Lcd_WriteData_16Bit(POINT_COLOR);
}
static inline void LCD_DrawPixel(uint16_t x, uint16_t y, uint16_t color)
{
    POINT_COLOR = color;
    LCD_DrawPoint(x, y);
}

// Read point
uint16_t LCD_ReadPoint(uint16_t x, uint16_t y) {
    uint16_t color;
    if (x >= lcddev.width || y >= lcddev.height) return 0;
    LCD_SetCursor(x, y);
    LCD_ReadRAM_Prepare();
    color = Lcd_ReadData_16Bit();
    return color;
}

// Clear screen
void LCD_Clear(uint16_t Color) {
    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
    for (uint32_t i = 0; i < lcddev.width * lcddev.height; i++) {
        Lcd_WriteData_16Bit(Color);
    }
}

// Fill screen
void LCD_FillScreen(uint16_t color) {
    uint8_t line[LCD_W * 2];
    for (int i = 0; i < LCD_W; i++) {
        line[2*i] = color >> 8;
        line[2*i + 1] = color & 0xFF;
    }
    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
    LCD_WriteRAM_Prepare();
    for (int y = 0; y < lcddev.height; y++) {
        for (int x = 0; x < LCD_W; x++) {
            Lcd_WriteData_16Bit(color);
        }
    }
}

// Write data buffer
void LCD_WriteDataBuffer(uint8_t *data, uint32_t length) {
    for (uint32_t i = 0; i < length; i++) {
        LCD_WR_DATA(data[i]);
    }
}

// Set display direction
void LCD_direction(uint8_t direction) {
    lcddev.setxcmd = 0x2A;
    lcddev.setycmd = 0x2B;
    lcddev.wramcmd = 0x2C;
    lcddev.rramcmd = 0x2E;
    switch (direction) {
        case 0: // Portrait
            lcddev.width = LCD_W;
            lcddev.height = LCD_H;
            LCD_WriteReg(0x36, (1 << 6) | (1 << 3)); // MY=1, BGR=1
            break;
        case 1: // Landscape (90 degrees)
            lcddev.width = LCD_H;
            lcddev.height = LCD_W;
            LCD_WriteReg(0x36, (1 << 3) | (1 << 4) | (1 << 5)); // MV=1, MX=1, BGR=1
            break;
        case 2: // Portrait (180 degrees)
            lcddev.width = LCD_W;
            lcddev.height = LCD_H;
            LCD_WriteReg(0x36, (1 << 3) | (1 << 7)); // MX=1, BGR=1
            break;
        case 3: // Landscape (270 degrees)
            lcddev.width = LCD_H;
            lcddev.height = LCD_W;
            LCD_WriteReg(0x36, (1 << 3) | (1 << 5) | (1 << 6) | (1 << 7)); // MY=1, MX=1, MV=1, BGR=1
            break;
        default:
            break;
    }
}

// Read ID
uint16_t LCD_Read_ID(void) {
    uint8_t val[4] = {0};
    LCD_ReadReg(0xD3, val, 4);
    return (val[2] << 8) | val[3];
}

// Draw line
void LCD_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int16_t dx = abs(x1 - x0);
    int16_t dy = abs(y1 - y0);
    int16_t sx = x0 < x1 ? 1 : -1;
    int16_t sy = y0 < y1 ? 1 : -1;
    int16_t err = dx - dy;
    int16_t e2;

    POINT_COLOR = color;
    while (1) {
    	LCD_DrawPixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) break;
        e2 = 2 * err;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// Draw rectangle
void LCD_DrawRectangle(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    if (x0 >= lcddev.width || y0 >= lcddev.height || x1 >= lcddev.width || y1 >= lcddev.height) return;
    if (x0 > x1) {
        uint16_t tmp = x0;
        x0 = x1;
        x1 = tmp;
    }
    if (y0 > y1) {
        uint16_t tmp = y0;
        y0 = y1;
        y1 = tmp;
    }
    LCD_SetWindows(x0, y0, x1, y1);
    for (uint32_t i = 0; i < (x1 - x0 + 1) * (y1 - y0 + 1); i++) {
        Lcd_WriteData_16Bit(color);
    }
}




void write16Color(uint16_t color, int32_t count) {
    while (count-- > 0) {
        Lcd_WriteData_16Bit(color); // Write 16-bit color value using 8-bit bus
    }
}

void fillRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	uint16_t y;
	LCD_SetAddrWindow(x0, y0, x1, y1);
    LCD_CS_CLR();
    LCD_WriteRAM_Prepare();
    for (y = y0; y <= y1; y++) {
         write16Color(color, x1 - x0 + 1); // Write pixels for one row
     }
     LCD_CS_SET();
}
// Delay functions
void delay_us(uint32_t us) {
    uint32_t start = HAL_GetTick();
    while ((HAL_GetTick() - start) * 1000 < us);
}

void delay_ms(uint32_t ms) {
    HAL_Delay(ms);
}





static inline void LCD_Write8(uint8_t data) {

    DATA_Port->ODR = (DATA_Port->ODR & 0xFF00) | data;
	LCD_WR_CLR();
	LCD_WR_SET();

}

static inline void LCD_Write8Register8(uint8_t a, uint8_t d) {
	LCD_RS_CLR();
	LCD_Write8(a);
	LCD_RS_SET();
	LCD_Write8(d);
}

static inline void LCD_Write32Register8(uint8_t a, uint32_t d) {
	LCD_RS_CLR();
	LCD_Write8(a);
	LCD_RS_SET();
	LCD_Write8(d >> 24);
	LCD_Write8(d >> 16);
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}



static inline void LCD_Write16Register16(uint16_t a, uint16_t d) {
	LCD_RS_CLR();
	LCD_Write8(a >> 8);
	LCD_Write8(a);
	LCD_RS_SET();
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}
static inline void LCD_Write24Register8(uint8_t a, uint32_t d) {
	LCD_RS_CLR();
	LCD_Write8(a);
	LCD_RS_SET();
	LCD_Write8(d >> 16);
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}
static inline void LCD_Write16Register8(uint8_t a, uint16_t d) {
	LCD_RS_CLR();
	LCD_Write8(a);
	LCD_RS_SET();
	LCD_Write8(d >> 8);
	LCD_Write8(d);
}



void LCD_SetAddrWindow(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2) {
	LCD_CS_CLR();
	LCD_Write32Register8(0x2A, (x1 << 16) | x2);
	LCD_Write32Register8(0x2B, (y1 << 16) | y2);
    LCD_CS_SET();
}



uint16_t m_width=LCD_H;
uint16_t m_height=LCD_W;

void LCD_DrawPixel1(uint16_t x, uint16_t y, uint16_t color) {
	// Clip
	if ((x < 0) || (y < 0) || (x >= m_width) || (y >= m_height)) return;

	LCD_CS_CLR();
        LCD_SetAddrWindow(x, y, m_width - 1, m_height - 1);
	LCD_CS_CLR();
	LCD_Write16Register8(0x2C, color);
        LCD_CS_SET();
      }

void LCD_DrawLine1(uint16_t x0, uint16_t y0, uint16_t x1, int16_t y1, uint16_t color) {
	// Bresenham's algorithm - thx wikpedia

	int16_t steep = abs(y1 - y0) > abs(x1 - x0);
	if (steep) {
		swap(x0, y0);
		swap(x1, y1);
	}

	if (x0 > x1) {
		swap(x0, x1);
		swap(y0, y1);
	}

	int16_t dx, dy;
	dx = x1 - x0;
	dy = abs(y1 - y0);

	int16_t err = dx / 2;
	int16_t ystep;

	if (y0 < y1) {
		ystep = 1;
	} else {
		ystep = -1;
	}

	for (; x0 <= x1; x0++) {
		if (steep) {
			LCD_DrawPixel1(y0, x0, color);
		} else {
			LCD_DrawPixel1(x0, y0, color);
		}
		err -= dy;
		if (err < 0) {
			y0 += ystep;
			err += dx;
		}
	}
}



#include "stm32f4xx_hal.h"
#include "ili9486.h"

// Assuming sFONT is defined in a header like "fonts.h"
extern sFONT Font24;
extern sFONT Font20;
extern sFONT Font16;
extern sFONT Font12;
extern sFONT Font8;

// Function to draw a string using the specified sFONT
void GUI_DrawFont(uint16_t x, uint16_t y, uint16_t fc, uint16_t bc, const char *s, sFONT *font, uint8_t mode) {
    uint16_t x0 = x;
    uint8_t char_width = font->Width;  // Width of each character
    uint8_t char_height = font->Height; // Height of each character
    uint8_t bytes_per_char = (char_width + 7) / 8 * char_height; // Bytes per character bitmap

    while (*s) { // Process each character in the string
        uint8_t c = *s; // Current character (ASCII)
        if (c < 32 || c > 126) { // Basic ASCII range check
            c = 32; // Replace invalid characters with space
        }

        // Calculate the offset in the font table for the character
        uint32_t offset = (c - 32) * bytes_per_char; // Assuming font starts at ASCII 32 (space)
        const uint8_t *char_data = font->table + offset; // Pointer to character's bitmap

        // Set the window for the current character
        LCD_SetWindows(x, y, x + char_width - 1, y + char_height - 1);

        // Render the character bitmap
        for (uint16_t row = 0; row < char_height; row++) {
            for (uint16_t col = 0; col < char_width; col++) {
                uint8_t byte_idx = (row * ((char_width + 7) / 8)) + (col / 8);
                uint8_t bit_mask = 0x80 >> (col % 8);
                uint8_t pixel = char_data[byte_idx] & bit_mask;

                if (!mode) { // Non-overlay mode (solid background)
                    if (pixel) {
                        Lcd_WriteData_16Bit(fc); // Foreground color
                    } else {
                        Lcd_WriteData_16Bit(bc); // Background color
                    }
                } else { // Overlay mode (transparent background)
                    if (pixel) {
                        POINT_COLOR = fc;
                        LCD_DrawPoint(x + col, y + row); // Draw only foreground pixels
                    }
                }
            }
        }

        // Move to the next character position
        x += char_width;
        if (x + char_width > lcddev.width) { // Check for screen boundary
            x = x0;
            y += char_height;
            if (y + char_height > lcddev.height) {
                break; // Exit if we exceed screen height
            }
        }
        s++; // Next character
    }

    // Restore full-screen window
    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
}

void customLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color){
int16_t x2=x0+1;
int16_t y2=(y1+y0)/2;
LCD_DrawLine(x0, y0, x2, y2,color);
LCD_DrawLine(x2, y2, x1, y1,color);


}
