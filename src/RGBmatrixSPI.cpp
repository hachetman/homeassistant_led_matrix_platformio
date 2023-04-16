#include "RGBmatrixSPI.h"
#include "stdint.h"

#include <stdlib.h>     /* malloc, free, rand */
#include <sys/types.h>

RGBmatrixSPI::RGBmatrixSPI(int16_t width, int16_t height, int speed)
    : Adafruit_GFX(width, height), matrix_width(width), matrix_height(height),
      spi_speed(speed), spi(VSPI) {
  Serial.println("Initializing");
  spi.begin();
  pinMode(VSPI_SS, OUTPUT);
  frameBuffer = (uint16_t*)malloc(width * height*4);
  transferBuffer =(uint8_t*)frameBuffer;
}
void RGBmatrixSPI::drawPixel(int16_t x, int16_t y, uint16_t c) {
  uint16_t address = (x + matrix_width * y);
  *(frameBuffer + address) = c;
}

void RGBmatrixSPI::fillScreen(uint16_t color){
  for (int i = 0; i < matrix_width*matrix_height; i++) {
    *(frameBuffer + i) = color;
  }
}

void RGBmatrixSPI::setBrightness(int8_t brightness) {
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::brightness));
  spi.transfer(brightness);
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  spi.endTransaction();
}
void RGBmatrixSPI::transfer() {
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::transfer));
  spi.transfer(0x00);
  spi.transfer(0x00);
  for (int i = 0; i < matrix_width*matrix_height*2; i++) {
    spi.transfer(*(frameBuffer+i));
  }

  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  spi.endTransaction();//
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::buffer_switch));
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer                               //
  spi.endTransaction();
  //safe this buffer for next scroll action
  for (int i = 0; i < matrix_width*matrix_height; i++) {
    *(frameBuffer + i + matrix_width*matrix_height) = *(frameBuffer + i);
  }
}

void RGBmatrixSPI::scroll() {

  for (int j = 0; j < matrix_width; j++) {
    spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
    digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
    spi.transfer(static_cast<uint8_t>(Registers::transfer));
    spi.transfer(0x00);
    spi.transfer(0x00);
    for (int row = 0; row < matrix_width; row++) {
      for (int column = 0; column < matrix_height; column++) {
        if (row < 20) {
          spi.transfer(*(transferBuffer+ (row * matrix_height + column)*2));
          spi.transfer(*(transferBuffer+ (row * matrix_height + column)*2 + 1));
        } else {
          if (column < j) {
            // new data
            spi.transfer(*(transferBuffer+ (row * matrix_height + column + 64 - j)*2));
            spi.transfer(*(transferBuffer+ (row * matrix_height + column + 64 - j)*2 + 1));
          } else {
            spi.transfer(*(transferBuffer+ (matrix_height*(matrix_width-1) +
                                            row * matrix_height + column + 64 - j)*2));
            spi.transfer(*(transferBuffer+ (matrix_height*(matrix_width-1) +
                                            row * matrix_height + column + 64 - j)*2 + 1));
          }
        }
      }
    }
    digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
    spi.endTransaction();//
    spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
    digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
    spi.transfer(static_cast<uint8_t>(Registers::buffer_switch));
    digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer                               //
    spi.endTransaction();
  }
  // safe this buffer for next scoll action
  for (int i = 0; i < matrix_width*matrix_height; i++) {
    *(frameBuffer + i + matrix_width*matrix_height) = *(frameBuffer + i);
  }
}
