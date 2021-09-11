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
  SPIClass spi;
  enum class Registers : std::uint8_t {
    brightness = 0xf1,
    transfer = 0xf3
  };
  const uint16_t lower_bytes = 0x00ff;
  const uint16_t upper_shift = 8;
};

#endif
