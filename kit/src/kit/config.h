#ifndef MF_CONFIG_H
#define MF_CONFIG_H

// ==========================================
// REQUIRED LIBRARIES ONLY
// ==========================================

#include <SPI.h>
#include <FS.h>
#include <Wire.h>
#include <SD.h>

#include <Audio.h>
#include <Arduino_GFX_Library.h>
#include <Adafruit_MLX90640.h>


// ==========================================
// SD CARD
// ==========================================

#define SD_SCK 12
#define SD_MISO 13
#define SD_MOSI 11
#define SD_CS 10


// ==========================================
// SPEAKER / I2S AUDIO
// ==========================================

#define I2S_DOUT 19
#define I2S_BCLK 20
#define I2S_LRCK 46


// ==========================================
// I2C
// Used by MLX90640 thermal camera
// ==========================================

#define I2C_SDA_PIN 17
#define I2C_SCL_PIN 18


// ==========================================
// MLX90640
// ==========================================

#define MLX_I2C_ADDR 0x33


#endif
