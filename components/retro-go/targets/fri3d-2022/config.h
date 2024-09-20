// Target definition
#define RG_TARGET_NAME             "FRI3D-2022"

// Storage
#define RG_STORAGE_DRIVER           1                   // 0 = Host, 1 = SDSPI, 2 = SDMMC, 3 = USB, 4 = Internal Flash
#define RG_STORAGE_SDSPI_HOST       SPI3_HOST           // Used by SDSPI and SDMMC
#define RG_STORAGE_SDSPI_SPEED      5000                // Used by SDSPI and SDMMC
#define RG_STORAGE_ROOT             "/sd"               // Storage mount point
#define RG_STORAGE_FLASH_PARTITION  "vfs"		// Fallback to internal flash partition

// Audio
#define RG_AUDIO_USE_BUZZER_PIN     32
#define RG_AUDIO_USE_INT_DAC        1   // 0 = Disable, 1 = GPIO25, 2 = GPIO26, 3 = Both
#define RG_AUDIO_USE_EXT_DAC        0   // 0 = Disable, 1 = Enable

// Video
#define RG_SCREEN_DRIVER            0   // 0 = ILI9341 but also works on ST7789 with below RG_SCREEN_INIT()
#define RG_SCREEN_HOST              SPI3_HOST
#define RG_SCREEN_SPEED             SPI_MASTER_FREQ_40M
#define RG_SCREEN_WIDTH             240
#define RG_SCREEN_HEIGHT            240
#define RG_SCREEN_ROTATE            0
#define RG_SCREEN_MARGIN_TOP        0
#define RG_SCREEN_MARGIN_BOTTOM     0
#define RG_SCREEN_MARGIN_LEFT       0
#define RG_SCREEN_MARGIN_RIGHT      0
#define ST7789_MADCTL_RGB 0x00
#define ST7789_MADCTL_BGR 0x08
#define RG_SCREEN_INIT()                                                                                         \
    ILI9341_CMD(0xCF, 0x00, 0xc3, 0x30);                                                                         \
    ILI9341_CMD(0xED, 0x64, 0x03, 0x12, 0x81);                                                                   \
    ILI9341_CMD(0xE8, 0x85, 0x00, 0x78);                                                                         \
    ILI9341_CMD(0xCB, 0x39, 0x2c, 0x00, 0x34, 0x02);                                                             \
    ILI9341_CMD(0xF7, 0x20);                                                                                     \
    ILI9341_CMD(0xEA, 0x00, 0x00);                                                                               \
    ILI9341_CMD(0xC0, 0x1B);                 /* Power control   //VRH[5:0] */                                    \
    ILI9341_CMD(0xC1, 0x12);                 /* Power control   //SAP[2:0];BT[3:0] */                            \
    ILI9341_CMD(0xC5, 0x32, 0x3C);           /* VCM control */                                                   \
    ILI9341_CMD(0xC7, 0x91);                 /* VCM control2 */                                                  \
    ILI9341_CMD(0x36, (0x00 | 0x00 | ST7789_MADCTL_RGB)); /* Memory Access Control - RGB */ \
    ILI9341_CMD(0x21, 0x80); /* Invert colors */ \
    ILI9341_CMD(0xB1, 0x00, 0x10);           /* Frame Rate Control (1B=70, 1F=61, 10=119) */                     \
    ILI9341_CMD(0xB6, 0x0A, 0xA2);           /* Display Function Control */                                      \
    ILI9341_CMD(0xF6, 0x01, 0x30);                                                                               \
    ILI9341_CMD(0xF2, 0x00); /* 3Gamma Function Disable */                                                       \
    ILI9341_CMD(0x26, 0x01); /* Gamma curve selected */                                                          \
    ILI9341_CMD(0xE0, 0xD0, 0x00, 0x05, 0x0E, 0x15, 0x0D, 0x37, 0x43, 0x47, 0x09, 0x15, 0x12, 0x16, 0x19); \
    ILI9341_CMD(0xE1, 0xD0, 0x00, 0x05, 0x0D, 0x0C, 0x06, 0x2D, 0x44, 0x40, 0x0E, 0x1C, 0x18, 0x16, 0x19);

// Fri3D Camp 2024 will need BGR: ILI9341_CMD(0x36, (0x00 | 0x00 | ST7789_MADCTL_BGR)); /* Memory Access Control */ 


// Input
// Refer to rg_input.h to see all available RG_KEY_* and RG_GAMEPAD_*_MAP types
// ADC1_CHANNEL_6 = GPIO34
// ADC2_CHANNEL_9 = GPIO26 is the same as on the working device

/*
    {RG_KEY_LEFT,  ADC_UNIT_2, ADC_CHANNEL_9, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_RIGHT, ADC_UNIT_2, ADC_CHANNEL_9, ADC_ATTEN_DB_11, -1, 1024},\

#define RG_GAMEPAD_ADC_MAP {\
    {RG_KEY_UP,    ADC_UNIT_1, ADC_CHANNEL_6, ADC_ATTEN_DB_11, 3072, 4096},\
    {RG_KEY_DOWN,  ADC_UNIT_1, ADC_CHANNEL_6, ADC_ATTEN_DB_11, -1, 1024},\
}

*/

// PIN USAGE:
// IO2: LED.DIN
// IO5: LCD.CS
// IO33: LCD.DC
// IO6-11 = Shared with Flash
// IO12 = MTDI , internal PD
// IO13: is on MENU which is the A button
// IO15 = MTDO , internal PU AND CHG.EN? dont use...
// IO16-17 shared with PSRAM
// IO25: IR.IN AND also grounding it turns on a LED on the back BUT works
// IO27: START button is E on the Funduino
// IO32: buzzer
// IO34: CHG.STATUS BUT it works because I'm not charging
// IO35: bat.mon adc doesnt seem to work (ADC1_CHANNEL_7) stuck at ~2500
// IO36: adc doesnt work, goes from 0 to 730 (accellerometer interrupt)
// IO39: triggers recovery mode "[warn] rg_system_init: Button 00000000 10000000 being held down..."

#define RG_RECOVERY_BTN RG_KEY_MENU

// USE:
// IO34 - 39 = Input only

/*
 Start button can be used if buzzer isn't used.  Mapping would be as follows:
        {RG_KEY_START, GPIO_NUM_32, GPIO_FLOATING,    1},

  Avoid using the Select button as the accelerometer interrupt interferes with is.  Without
  custom code the Select button is stuck on low.

  In the mapping below, we use the following assignments:
  Menu = left shoulder button
  Start = right shoulder button
*/

#define RG_GAMEPAD_GPIO_MAP                               \
    {                                                     \
        {RG_KEY_MENU,  GPIO_NUM_27, GPIO_PULLUP_ONLY, 0}, \
        {RG_KEY_START, GPIO_NUM_14, GPIO_PULLUP_ONLY, 0}, \
        {RG_KEY_A,     GPIO_NUM_13, GPIO_PULLUP_ONLY, 0}, \
        {RG_KEY_B,     GPIO_NUM_12, GPIO_PULLUP_ONLY, 0}, \
        {RG_KEY_UP,    GPIO_NUM_39, GPIO_PULLUP_ONLY, 0}, \
        {RG_KEY_DOWN,  GPIO_NUM_15, GPIO_PULLUP_ONLY, 0}, \
        {RG_KEY_LEFT,  GPIO_NUM_26, GPIO_PULLUP_ONLY, 0}, \
        {RG_KEY_RIGHT, GPIO_NUM_0,  GPIO_PULLUP_ONLY, 0}, \
}

/*
[warn] rg_system_init: Button 00000010 00000010 being held down...                                                                                                                                                
    {RG_KEY_SELECT, GPIO_NUM_25, GPIO_PULLUP_ONLY, 0},\
    {RG_KEY_OPTION, GPIO_NUM_25, GPIO_PULLUP_ONLY, 0},\

        "None", "None", "None", "None", "R", "L", "X", "A", "Right", "Left", "Down", "Up", "Start", "Select", "Y", "B"

typedef enum
{
    RG_KEY_UP      = (1 << 0),
    RG_KEY_RIGHT   = (1 << 1),
    RG_KEY_DOWN    = (1 << 2),
    RG_KEY_LEFT    = (1 << 3),
    RG_KEY_SELECT  = (1 << 4),
    RG_KEY_START   = (1 << 5),
    RG_KEY_MENU    = (1 << 6),
    RG_KEY_OPTION  = (1 << 7),

    RG_KEY_A       = (1 << 8),
    RG_KEY_B       = (1 << 9),
    RG_KEY_X       = (1 << 10),
    RG_KEY_Y       = (1 << 11),
    RG_KEY_L       = (1 << 12),
    RG_KEY_R       = (1 << 13),
    RG_KEY_COUNT   = 14,
    RG_KEY_ANY     = 0xFFFF,
    RG_KEY_ALL     = 0xFFFF,
    RG_KEY_NONE    = 0,
} rg_key_t;


[warn] rg_system_init: Button 00000000 00000010 being held down... this is the RIGHT key!
*/

// In the launcher, RG_KEY_MENU and RG_KEY_START both cycle through apps.
// RG_KEY_OPTION  doesnt do anything?
// RG_KEY_SELECT goes to previous emulator (also UP and LEFT) - not needed

// RG_KEY_MENU opens up the retro-go menu (if RG_KEY_SELECT, GPIO_NUM_25 then it will go to the next but that's because this one pin 25 has interference!)
// RG_KEY_START goes to next emulator (also DOWN and RIGHT) and is on E

// In doom:
// RG_KEY_SELECT doesnt do much?
// RG_KEY_MENU opens up the doom menu
// RG_KEY_OPTION probably opens the retro-go menu?

// RG_KEY_A is shoot and select


#define RG_BATTERY_DRIVER           1
//#define RG_BATTERY_ADC_UNIT         ADC_UNIT_1
//#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_6
#define RG_BATTERY_ADC_UNIT         ADC_UNIT_1
#define RG_BATTERY_ADC_CHANNEL      ADC_CHANNEL_7
#define RG_BATTERY_CALC_PERCENT(raw) (((raw) * 2.f - 3500.f) / (4200.f - 3500.f) * 100.f)
#define RG_BATTERY_CALC_VOLTAGE(raw) ((raw) * 2.f * 0.001f)



// Status LED for disk activity
//#define RG_GPIO_LED                 GPIO_NUM_2


// SPI Display
// Fri3d badge 2022 SPI uses the IOMUX pins for SPI3_HOST, so for "purity" we use SPI3_HOST.
// SPI2_HOST works as well, pins are then mapped through the GPIO matrix (makes no difference
// in practice at the speed we're using here).
#define RG_GPIO_LCD_MISO            GPIO_NUM_19
#define RG_GPIO_LCD_MOSI            GPIO_NUM_23
#define RG_GPIO_LCD_CLK             GPIO_NUM_18
#define RG_GPIO_LCD_CS              GPIO_NUM_5
#define RG_GPIO_LCD_DC              GPIO_NUM_33

// SPI SD Card
#define RG_GPIO_SDSPI_MISO          RG_GPIO_LCD_MISO
#define RG_GPIO_SDSPI_MOSI          RG_GPIO_LCD_MOSI
#define RG_GPIO_SDSPI_CLK           RG_GPIO_LCD_CLK
#define RG_GPIO_SDSPI_CS            GPIO_NUM_4

