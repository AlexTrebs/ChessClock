#include <stdint.h>

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

class display {
  public:
    display(int8_t cs, int8_t dc, int8_t mosi, int8_t sclk,
                    int8_t rst = -1);
    display(int8_t cs, int8_t dc, int8_t rst);
    
    void setRotation(uint8_t m);
    void init(uint16_t width, uint16_t height, uint8_t spiMode = SPI_MODE0);

    protected:
  uint8_t _colstart2 = 0, ///< Offset from the right
      _rowstart2 = 0;     ///< Offset from the bottom

  private:
    uint16_t windowWidth;
    uint16_t windowHeight;
};