#include "RGBmatrixSPI.h"

#include <cstdint>
#include <cstdlib>

RGBmatrixSPI::RGBmatrixSPI(int speed, int rotation)
    : Adafruit_GFX(MATRIX_WIDTH, MATRIX_HEIGHT), spi_speed(speed), spi(VSPI),
      rotation(rotation) {
  Serial.println("Initializing");
  spi.begin();
  pinMode(VSPI_SS, OUTPUT);
  frameBuffer = (uint16_t*)malloc(MATRIX_HEIGHT * MATRIX_WIDTH*4);
  backBuffer = (uint16_t*)malloc(MATRIX_HEIGHT * MATRIX_WIDTH*2);
  transferBuffer =(uint8_t*)backBuffer;
}
void RGBmatrixSPI::drawPixel(int16_t x, int16_t y, uint16_t c) {
  uint16_t address = (x + MATRIX_WIDTH * y);
  *(frameBuffer + address) = c;
}

void RGBmatrixSPI::fillScreen(uint16_t color){
  for (int i = 0; i < MATRIX_WIDTH*MATRIX_HEIGHT; i++) {
    *(frameBuffer + i) = color;
  }
}

void RGBmatrixSPI::fillClock(uint16_t color) const{
  for (int i = 0; i < MATRIX_WIDTH*20; i++) {
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
  uint16_t x_rot, y_rot;
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      switch (rotation) {
      case 0:
        x_rot = x;
        y_rot = y;
        break;
      case 1:
        x_rot = MATRIX_WIDTH - 1 - y;
        y_rot = x;
        break;
      case 2:
        x_rot = MATRIX_WIDTH - 1 - x;
        y_rot = MATRIX_HEIGHT - 1 - y;
        break;
      case 3:
        x_rot = y;
        y_rot = MATRIX_HEIGHT - 1 - x;
        break;
      }
      backBuffer[x_rot + y_rot * MATRIX_WIDTH] = frameBuffer[x + y * MATRIX_WIDTH];
    }
  }
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::transfer));
  spi.transfer(0x00);
  spi.transfer(0x00);
  for (int i = 0; i < (MATRIX_WIDTH*MATRIX_HEIGHT << 1); i++) {
    spi.transfer(transferBuffer[i]);
  }
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  spi.endTransaction();//
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::buffer_switch));
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer                               //
  spi.endTransaction();
  //safe this buffer for next scroll action
  for (int i = 0; i < MATRIX_WIDTH*MATRIX_HEIGHT; i++) {
    *(frameBuffer + i + MATRIX_WIDTH*MATRIX_HEIGHT) = *(frameBuffer + i);
  }
}

void RGBmatrixSPI::scroll( uint32_t scroll) {
  uint16_t x_rot, y_rot;
  for (int y = 0; y < MATRIX_HEIGHT; y++) {
    for (int x = 0; x < MATRIX_WIDTH; x++) {
      switch (rotation) {
      case 0:
        x_rot = x;
        y_rot = y;
        break;
      case 1:
        x_rot = MATRIX_WIDTH - 1 - y;
        y_rot = x;
        break;
      case 2:
        x_rot = MATRIX_WIDTH - 1 - x;
        y_rot = MATRIX_HEIGHT - 1 - y;
        break;
      case 3:
        x_rot = y;
        y_rot = MATRIX_HEIGHT - 1 - x;
        break;
      }
      if (y < 20) {
        backBuffer[y_rot * MATRIX_HEIGHT + x_rot] = frameBuffer[y * MATRIX_HEIGHT + x];
      } else {
        if (x < scroll) {
          // new data
          backBuffer[y_rot * MATRIX_HEIGHT + x_rot] = frameBuffer[y * MATRIX_HEIGHT + x + 64 - scroll];
        } else {
          backBuffer[y_rot * MATRIX_HEIGHT + x_rot] = frameBuffer[(MATRIX_HEIGHT*(MATRIX_WIDTH-1) +
                                                           y * MATRIX_HEIGHT + x + 64 - scroll)];
        }
      }
    }
  }
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::transfer));
  spi.transfer(0x00);
  spi.transfer(0x00);
  for (int i = 0; i < (MATRIX_WIDTH*MATRIX_HEIGHT << 1); i++) {
    spi.transfer(*(transferBuffer+i));
  }
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer
  spi.endTransaction();//
  spi.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE0));
  digitalWrite(VSPI_SS, LOW); // pull SS slow to prep other end for transfer
  spi.transfer(static_cast<uint8_t>(Registers::buffer_switch));
  digitalWrite(VSPI_SS, HIGH); // pull ss high to signify end of data transfer                               //
  spi.endTransaction();
}
