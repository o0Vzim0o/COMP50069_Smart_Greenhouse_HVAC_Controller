#ifndef CONFIG_H
#define CONFIG_H

// ======================================================
// SENSOR AND INPUT PINS
// ======================================================

// DHT22 temperature and humidity sensor
#define DHT_PIN 23

// Potentiometer analogue input for ADC testing
#define POT_PIN 34

// LDR analogue input for day/night detection
#define LDR_PIN 32

// ======================================================
// RGB STATUS LED PINS
// ======================================================

// Red LED indicates danger / emergency
#define RED_PIN 13

// Green LED indicates safe operation
#define GREEN_PIN 12

// Blue LED indicates warning state
#define BLUE_PIN 14

// ======================================================
// ALARM AND EMERGENCY INPUT
// ======================================================

// Buzzer alarm output
#define BUZZER_PIN 27

// Emergency override push button
#define BUTTON_PIN 15

// ======================================================
// OLED DISPLAY CONFIGURATION
// ======================================================

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

// ======================================================
// ADDITIONAL I ADDED
// ======================================================

#define START_BUTTON_PIN 4

#endif