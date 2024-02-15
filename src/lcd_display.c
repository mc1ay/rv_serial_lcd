/*
 * SerialDisplay.c
 *
 *  Original author: Hoang Ta
 *
 *  Modified by Mitchell Clay to add support for different size screens
 *  and to handle carriage return and non-printable characters
 *
 *  UART configured to 9600, 8, 0, 1
 */

#include <stdio.h>
#include <ctype.h>
#include "LiquidCrystal_I2C.h"
#include "metal/uart.h"

#define LCD_WIDTH  20
#define LCD_HEIGHT  4

struct metal_i2c *lcd_i2c;
struct metal_uart *uart;
static int first = 1;

int main(void)
{
	int baud = 0;
	char str[16] = {0};
	char lines[LCD_WIDTH][LCD_HEIGHT] = {0};
	int c, index = 0;
	int currentline = 0;
	char blankspace = 32;

	lcd_i2c = metal_i2c_get_device(0);
	if (lcd_i2c == NULL) {
		printf("I2C not available \n");
		return -1;
	}

	// Set the LCD address to 0x27
	LiquidCrystal_I2C_init(lcd_i2c, 0x27, 20, 4, LCD_5x8DOTS);
	LiquidCrystal_I2C_begin(); 	// initialize the LCD

	// Turn on the backlight and print a message.
	LiquidCrystal_I2C_backlight();

	// Initialize the serial port at a default speed of 115200 baud
	uart = metal_uart_get_device(0); // COM port
	if(uart == NULL) {
		printf("UART not available \n");
		return -1;
	}

	printf("Warning: set serial speed of terminal to 9600\r\n");
	metal_uart_init(uart, 9600);
	baud = metal_uart_get_baud_rate(uart);
	printf("baudrate %d\r\n", baud);
	LiquidCrystal_I2C_print("Baud rate:");
	LiquidCrystal_I2C_setCursor(0, 1); // set cursor to bottom left
	sprintf(str, "%d", baud);
	LiquidCrystal_I2C_print(str);

	printf("LCD width: %d\n", LCD_WIDTH);
	printf("LCD height: %d\n", LCD_HEIGHT);

	while (1)
	{
		// If characters arrived over the serial port...
		metal_uart_getc(uart, &c);
		//if (c != -1)
		if (c > 0)
		{
			// Clear LCD screen before print first character
			if(first)
			{
				LiquidCrystal_I2C_clear();  // Clear the screen
				first = 0;  // clear flag
				LiquidCrystal_I2C_setCursor(0, 0); // set cursor to top left
			}

			// Handle carriage return
			if (c == 13) {
				for (int i = index; i < LCD_WIDTH; i++) {
					lines[index % LCD_WIDTH][currentline] = blankspace;
			        index++;
				}
			}
			// Printable characters
			else if (isprint(c)){
				lines[index % LCD_WIDTH][currentline] = c;
				LiquidCrystal_I2C_write(c); // print character to LCD
				index++;
			}

			// Check for end of line, move to next line if not
			// at the bottom line of the display
			if(index == LCD_WIDTH)
			{
				if (currentline < LCD_HEIGHT - 1) {
					currentline++;
					LiquidCrystal_I2C_setCursor(0, currentline);
					index = 0;
				}
				// If on bottom line, move lines up and start a new line
				else if (currentline == LCD_HEIGHT - 1){
					for (int i = 0; i < LCD_HEIGHT - 1; i++) {
						LiquidCrystal_I2C_setCursor(0, i);
						for (int j = 0; j < LCD_WIDTH; j++) {
							lines[j][i] = lines[j][i + 1];
							LiquidCrystal_I2C_write(lines[j][i]);
						}
					}
					LiquidCrystal_I2C_setCursor(0, LCD_WIDTH - 1);
					for (int i = 0; i < LCD_WIDTH; i++) {
						LiquidCrystal_I2C_write(blankspace);
					}
				    LiquidCrystal_I2C_setCursor(0, LCD_WIDTH - 1);
				    index = 0;
				}
			}
		}
	}

	return 0;
}
