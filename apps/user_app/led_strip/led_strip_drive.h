
#ifndef led_strip_drive_h
#define led_strip_drive_h

#include "board_ac632n_demo_cfg.h"
#include "asm/ledc.h"
#include "asm/gpio.h"
// #define MIC_PIN     IO_PORTA_08
#define LEDC_PIN    IO_PORTA_07
#define LEDC1_PIN   IO_PORTA_08


typedef enum
{
    OFF,    //mic关闭
    ON,     //mic打开
}MIC_OFFON;

extern MIC_OFFON MIC_ENABLE;

void led_state_init(void);


void ledc_init(const struct ledc_platform_data *arg); 

#endif







