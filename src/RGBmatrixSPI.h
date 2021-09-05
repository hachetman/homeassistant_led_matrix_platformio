#ifndef RGBMATRIXSPI_H
#define RGBMATRIXSPI_H
#include "Adafruit_GFX.h"
#include "stdint.h"
#include <SPI.h>
#define VSPI_MISO MISO
#define VSPI_MOSI MOSI
#define VSPI_SCLK SCK
#define VSPI_SS SS

class RGBmatrixSPI : public Adafruit_GFX {
public:
  RGBmatrixSPI(int width, int height, int speed);
  void drawPixel(int16_t x, int16_t y, uint16_t c);
  void setBrightness(int8_t brightness);

private:
  int matrix_width;
  int matrix_height;
  int spi_speed;
  SPIClass *vspi = NULL;
};

#endif
