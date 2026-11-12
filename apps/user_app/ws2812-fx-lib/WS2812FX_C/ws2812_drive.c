#include "app_config.h"
#include "jiffies.h"
#include "ws2812_bsp.h"
#include "debug.h"
#include "my_effect.h"
#include "led_strand_effect.h"

extern void ledc_send_rgbbuf_isr(u8 index, u8 * rgbbuf, u32 led_num, u16 again_cnt);

/* 七彩灯使用第一路的第零段数据 */
/* 第一路幻彩，使用第一段的数据 */
static const u16 huancai1_num = (USER_WS2812_SEGMENT_1_LEN * 3);
static const u16 huancai2_num = (USER_WS2812_SEGMENT_2_LEN * 3);

#define WS2812_LED_BUF_SEC   __attribute__((section(".ws2812_led_buf"), aligned(4)))

static unsigned long tick_ms;
static u8 led_val_buf1[huancai1_num] WS2812_LED_BUF_SEC;
static u8 led_val_buf2[huancai2_num] WS2812_LED_BUF_SEC;


/************************************************** LEDC1(幻彩2) 单灯定位测试 *****************************************************/
/* 置 1：只点亮 LEDC1 上的第 WS2812_LEDC1_TEST_LED 个灯，其余灯和 RGB 灯全部熄灭 */
/* 测试方法：把 WS2812_LEDC1_TEST_LED 依次改成 0 ~ (USER_WS2812_SEGMENT_2_LEN - 1) 编译，核对灯序是否正确 */
/* 测试完成后把 WS2812_LEDC1_SINGLE_TEST_ENABLE 改回 0 即恢复原有效果 */
#define WS2812_LEDC1_SINGLE_TEST_ENABLE   0
#define WS2812_LEDC1_TEST_LED             (83)         // 灯序号，0 为第一颗
#define WS2812_LEDC1_TEST_R               0xFF      // 亮度 0 ~ 255
#define WS2812_LEDC1_TEST_G               0xFF
#define WS2812_LEDC1_TEST_B               0xFF
/*******************************************************************************************************************************/

#if WS2812_LEDC1_SINGLE_TEST_ENABLE
/* 只点亮 rgb_buf 中第 WS2812_LEDC1_TEST_LED 个灯，缓冲区顺序为 G,R,B */
static void ws2812_ledc1_single_test(u8 *rgb_buf, u16 buf_len)
{
    u16 led_num = buf_len / 3;
    u16 idx = ((u16)WS2812_LEDC1_TEST_LED % led_num) * 3;

    memset(rgb_buf, 0x00, buf_len);
    rgb_buf[idx + 0] = WS2812_LEDC1_TEST_G;
    rgb_buf[idx + 1] = WS2812_LEDC1_TEST_R;
    rgb_buf[idx + 2] = WS2812_LEDC1_TEST_B;
}
#endif


void ws281x_init()
{
}

void ws281x_show(unsigned char *pixels_pattern, unsigned short pattern_size)
{
    //pattern_size == fc_effect.led_num * 3  because chose NEO_GRB
    unsigned short i = 0;
    unsigned short k = 0;

    /* RGB驱动 ******************************************************************************************************/
    uint32_t rgb_offset = 0;
    uint8_t rgb_value[3];

    memset(rgb_value, 0x00, sizeof(rgb_value));
    if (fc_effect.rgbw_on_off_flag != DEVICE_OFF)
    {
        rgb_value[0] = (pixels_pattern[rgb_offset + 0] * fc_effect.static_mode_brightness) >> 3; // (>>3) --> (/8);
        rgb_value[1] = (pixels_pattern[rgb_offset + 1] * fc_effect.static_mode_brightness) >> 3;
        rgb_value[2] = (pixels_pattern[rgb_offset + 2] * fc_effect.static_mode_brightness) >> 3;
    }

    extern void fc_driver(u8 r, u8 g, u8 b);
#if WS2812_LEDC1_SINGLE_TEST_ENABLE
    rgb_value[0] = rgb_value[1] = rgb_value[2] = 0;   /* 单灯测试：RGB 灯熄灭 */
#endif
    // fc_driver(*pixels_pattern * fc_effect.static_mode_brightness / 8, * (pixels_pattern + 1) * fc_effect.static_mode_brightness / 8, * (pixels_pattern + 2) * fc_effect.static_mode_brightness / 8);
    fc_driver(rgb_value[0], rgb_value[1], rgb_value[2]);


    /* 幻彩1驱动 ******************************************************************************************************/
    uint32_t ledc0_offset = 3;

    memset(led_val_buf1, 0x00, sizeof(led_val_buf1));
    if (fc_effect.led_on_off_flag != DEVICE_OFF)
    {
#if 0
        uint8_t curr;
#endif
#if WHITE_LIGHT_EX_ENABLE
        if (user_ex_white_light_get_power())  // 如果白光灯打开，原幻彩灯被覆盖
        {
            for (i = 0; i < huancai1_num; i++)
            {
                led_val_buf1[i] = 0xFF * user_ex_white_light_get_level() / WHITE_LIGHT_MAX_LEVEL;
            }
        }
        else
#endif
        {
            if (fc_effect.Now_state == IS_STATIC)
            {   // 静态时，受到静态亮度影响，需要独立计算，库自带的亮度不适用实际方案
                for (i = 0; i < huancai1_num; i++)
                {
#if 0
                    // 静态色颜色或亮度变化时，渐变过度
                    curr = (pixels_pattern[ledc0_offset + i] * fc_effect.static_mode_brightness) >> 3; // (>>3) --> (/8);
                    if (led_val_buf1[i] > curr)
                        led_val_buf1[i]--;
                    else if (led_val_buf1[i] < curr)
                        led_val_buf1[i]++;
#else
                    led_val_buf1[i] = (pixels_pattern[ledc0_offset + i] * fc_effect.static_mode_brightness) >> 3; // (>>3) --> (/8);
#endif
                }
            }
            else
            {
                memcpy(led_val_buf1, pixels_pattern + ledc0_offset, huancai1_num);
            }
        }
    }
#if WS2812_LEDC1_SINGLE_TEST_ENABLE
    memset(led_val_buf1, 0x00, sizeof(led_val_buf1));   /* 单灯测试：幻彩1熄灭 */
#endif
    ledc_send_rgbbuf_isr(0, led_val_buf1, huancai1_num / 3, 0);


    /* 幻彩2驱动 ******************************************************************************************************/
    uint32_t ledc1_offset = 3 + huancai1_num;

    memset(led_val_buf2, 0x00, sizeof(led_val_buf2));
    if (fc_effect.led1_on_off_flag != DEVICE_OFF)
    {
#if 0
        uint8_t curr;
#endif
#if 0//WHITE_LIGHT_EX_ENABLE
        if (user_ex_white_light_get_power())  // 如果白光灯打开，原幻彩灯被覆盖
        {
            for (i = 0; i < huancai2_num; i++)
            {
                led_val_buf2[i] = 0xFF * user_ex_white_light_get_level() / WHITE_LIGHT_MAX_LEVEL;
            }
        }
        else
#endif
        {
            if (fc_effect.Now_state == IS_STATIC)
            {   // 静态时，受到静态亮度影响，需要独立计算，库自带的亮度不适用实际方案
                for (i = 0; i < huancai2_num; i++)
                {
#if 0
                // 静态色颜色或亮度变化时，渐变过度
                    curr = (pixels_pattern[ledc1_offset + i] * fc_effect.static_mode_brightness) >> 3; // (>>3) --> (/8);
                    if (led_val_buf2[i] > curr)
                        led_val_buf2[i]--;
                    else if (led_val_buf2[i] < curr)
                        led_val_buf2[i]++;
#else
                    led_val_buf2[i] = (pixels_pattern[ledc1_offset + i] * fc_effect.static_mode_brightness) >> 3; // (>>3) --> (/8);
#endif
                }
            }
            else
            {
                memcpy(led_val_buf2, pixels_pattern + ledc1_offset, huancai2_num);
            }
        }
    }
#if WS2812_LEDC1_SINGLE_TEST_ENABLE
    ws2812_ledc1_single_test(led_val_buf2, sizeof(led_val_buf2));   /* 单灯测试：只点亮第 x 颗灯 */
#endif
    ledc_send_rgbbuf_isr(1, led_val_buf2, huancai2_num / 3, 0);

    
    // printf("pixel-start\n");
    // put_buf(led_val_buf1, huancai1_num);
    // put_buf(led_val_buf2, huancai2_num);
    // printf("pixel-end");
}

unsigned long HAL_GetTick(void)
{
    // return tick_ms;
    return jiffies_msec();
}

// 每10ms调用一次
void run_tick_per_10ms(void)
{
    // tick_ms += 10;
    tick_ms = jiffies_msec();
}
