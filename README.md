# 📱 ILI9486 TFT LCD Driver for STM32

This repository contains a custom low-level driver implementation for the **ILI9486** 3.5" TFT LCD display, designed specifically for **STM32 microcontrollers** using an **8-bit parallel interface (8080 style)**.

---

## 🚀 Features

- ✅ 8-bit parallel interface (GPIO-based)
- ✅ Compatible with STM32 HAL (STM32CubeMX-generated projects)
- ✅ Basic graphics functions (pixel, line, fill, text)
- ✅ Fast GPIO writes using direct register access (optional)
- ✅ Tested on STM32F4 series

- ## 📁 Directory Structure

ILI9486_STM32/
├── Src/
│ └── ili9486.c # Main driver source file (contains LCD_Init)
├── Inc/
│ ├── ili9486.h # Driver header file
│ └── fonts/ 


## 🧰 GPIO Configuration

To use this driver, **wire the LCD’s D0–D7 data pins to one full GPIO port**, such as `GPIOD` (PD0–PD7), and connect the control signals as defined below:

### 🧷 Required Pin Mapping (you can modify mapping by editing definitions in ili9486.c)



| LCD Signal | STM32 Pin   | Description        |
|------------|-------------|--------------------|
| D0–D7      | PD0–PD7     | 8-bit Data Bus     |
| RESET      | PA8         | Reset Pin          |
| CS         | PA9         | Chip Select        |
| RS (DC)    | PA10        | Register Select    |
| WR         | PA11        | Write Strobe       |
| RD         | PA12        | Read Strobe        |
