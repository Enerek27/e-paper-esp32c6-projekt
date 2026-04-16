#ifndef _DEV_CONFIG_H_
#define _DEV_CONFIG_H_

#include <stdint.h>
#include <stdbool.h>

/* -- Pin definitions (upravi podľa svojho zapojenia) -- */
#ifndef EPD_MOSI_PIN
#define EPD_MOSI_PIN   18
#endif
#ifndef EPD_SCLK_PIN
#define EPD_SCLK_PIN   19
#endif
#ifndef EPD_CS_PIN
#define EPD_CS_PIN     1
#endif
#ifndef EPD_DC_PIN
#define EPD_DC_PIN     21
#endif
#ifndef EPD_RST_PIN
#define EPD_RST_PIN    22
#endif
#ifndef EPD_BUSY_PIN
#define EPD_BUSY_PIN   2
#endif


typedef uint8_t  UBYTE;
typedef uint16_t UWORD;
typedef uint32_t UDOUBLE;


int DEV_Module_Init(void);
void DEV_Module_Exit(void);

void DEV_Delay_ms(UDOUBLE xms);
void DEV_Digital_Write(uint8_t Pin, uint8_t Value);
int  DEV_Digital_Read(uint8_t Pin);

void DEV_SPI_WriteByte(UBYTE Data);
void DEV_SPI_Write_nByte(UBYTE *pData, UWORD Len);

#endif