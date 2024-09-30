#include <stdint.h>

#define SPI_MODE0 0x00
#define SPI_MODE1 0x04
#define SPI_MODE2 0x08
#define SPI_MODE3 0x0C

class display {
  public:
    display(uint16_t w, uint16_t h, int8_t cs, int8_t dc, int8_t mosi, int8_t sclk,
                    int8_t rst = -1);
    display(uint16_t w, uint16_t h, int8_t cs, int8_t dc, int8_t rst);
    void setRotation(uint8_t m);
    void init(uint16_t width, uint16_t height, uint8_t spiMode = SPI_MODE0);

    void setPixel(uint16_t index, uint16_t colour, uint16_t* bitmap);
    void drawPixel(uint16_t x, uint16_t y, uint16_t colour, uint16_t* bitmap);

  protected:
    uint8_t _colstart2 = 0, ///< Offset from the right
        _rowstart2 = 0;     ///< Offset from the bottom

  private:
    uint16_t windowWidth;
    uint16_t windowHeight;

    int8_t _rst;             ///< Reset pin # (or -1)
    int8_t _cs;              ///< Chip select pin # (or -1)
    int8_t _dc;              ///< Data/command pin #
    int8_t _mosi;            ///< MOSI pin # (or -1)
    int8_t _sclk;            ///< sclk select pin # (or -1)

};