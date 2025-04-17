#ifndef RGBMATRIXSPI_H
#define RGBMATRIXSPI_H
#include "Adafruit_GFX.h"
#include <SPI.h>
#define VSPI_SS SS
#define MATRIX_HEIGHT 64
#define MATRIX_WIDTH 64

class RGBmatrixSPI : public Adafruit_GFX {

public:
  RGBmatrixSPI(int speed, int rotation);
  void drawPixel(int16_t x, int16_t y, uint16_t c) override;
  void setBrightness(int8_t brightness);
  void fillScreen(uint16_t color) override;
  void fillClock(uint16_t color) const;

  uint16_t *frameBuffer;
  uint16_t *backBuffer;
  uint8_t *transferBuffer;
  void transfer();
  void scroll(uint32_t scroll);

private:
  const int spi_speed;
  SPIClass spi;
  int rotation = 0;
  enum class Registers : std::uint8_t {
    brightness = 0xf1,
    transfer = 0xf3,
    buffer_switch = 0xf4
  };
};

#endif
