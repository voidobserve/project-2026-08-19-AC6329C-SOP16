/*
- led_strip_drive.c
- 幻彩灯带驱动
- 麦克风驱动
 */
#include "led_strip_sys.h"
#include "led_strip_drive.h"
#include "asm/adc_api.h"
#include "asm/mcpwm.h"
#include "led_strand_effect.h"
MIC_OFFON MIC_ENABLE;           //0-关闭麦克风，1-开启麦克风

const struct ledc_platform_data ledc_data =
{
    .index = 0,         //控制器号
    .port = LEDC_PIN,//输出引脚
    .idle_level = 0,    //当前帧的空闲电平，0：低电平， 1：高电平
    .out_inv =0,       //起始电平，0：高电平开始， 1：低电平开始
    .bit_inv = 1,       //取数据时高低位镜像，0：不镜像，1：8位镜像，2:16位镜像，3:32位镜像
    .t_unit = t_42ns,   //时间单位
    // .t1h_cnt = 21,      //1码的高电平时间 = t1h_cnt * t_unit;21*42=882
    // .t1l_cnt = 8,       //1码的低电平时间 = t1l_cnt * t_unit;7*42=294
    // .t0h_cnt = 10,       //0码的高电平时间 = t0h_cnt * t_unit;8*42=336
    // .t0l_cnt = 20,      //0码的低电平时间 = t0l_cnt * t_unit;*/30*42=1260·
    .t1h_cnt = 16,
    .t1l_cnt = 7,
    .t0h_cnt = 7,
    .t0l_cnt = 16,

    .t_rest_cnt = 20000,//复位信号时间 = t_rest_cnt * t_unit;20000*42=840000
    .cbfun = NULL,      //中断回调函数
};

const struct ledc_platform_data ledc_data1 =
{
    .index = 1,         //控制器号
    .port = LEDC1_PIN,  //输出引脚
    .idle_level = 0,    //当前帧的空闲电平，0：低电平， 1：高电平
    .out_inv =0,        //起始电平，0：高电平开始， 1：低电平开始
    .bit_inv = 1,       //取数据时高低位镜像，0：不镜像，1：8位镜像，2:16位镜像，3:32位镜像
    .t_unit = t_42ns,   //时间单位
    // .t1h_cnt = 21,      //1码的高电平时间 = t1h_cnt * t_unit;21*42=882
    // .t1l_cnt = 8,       //1码的低电平时间 = t1l_cnt * t_unit;7*42=294
    // .t0h_cnt = 10,      //0码的高电平时间 = t0h_cnt * t_unit;8*42=336
    // .t0l_cnt = 20,      //0码的低电平时间 = t0l_cnt * t_unit;*/30*42=1260·
    .t1h_cnt = 16,
    .t1l_cnt = 7,
    .t0h_cnt = 7,
    .t0l_cnt = 16,

    .t_rest_cnt = 20000,//复位信号时间 = t_rest_cnt * t_unit;20000*42=840000
    .cbfun = NULL,      //中断回调函数
};

void led_state_init(void)
{
    #ifdef MY_DEBUG
    printf("-----------------led_state_init--------------------");
    #endif
    ledc_init(&ledc_data);
    ledc_init(&ledc_data1);

}

void led_gpio_init(void)
{
    //R
	gpio_set_die(IO_PORTA_01, 1);
	gpio_direction_output(IO_PORTA_01, 0);
    //G
	gpio_set_die(IO_PORTA_02, 1);
	gpio_direction_output(IO_PORTA_02, 0);
    //B
	gpio_set_die(IO_PORTB_05, 1);
	gpio_direction_output(IO_PORTB_05, 0);


}


//七彩灯pwm初始化
void led_pwm_init(void)
{

    // R
    struct pwm_platform_data pwm_p_data;
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch0;                        //通道号
    pwm_p_data.frequency = 1000;                            //1KHz
    pwm_p_data.duty = 0;                                    //上电输出0%占空比
    pwm_p_data.h_pin = IO_PORTA_01;                         //任意引脚
    pwm_p_data.l_pin = -1;                                  //任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                        //两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
    // G
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch1;                        //通道号
    pwm_p_data.frequency = 1000;                            //1KHz
    pwm_p_data.duty = 0;                                    //上电输出0%占空比
    pwm_p_data.h_pin = IO_PORTA_02 ;                         //任意引脚
    pwm_p_data.l_pin = -1;                                  //任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                        //两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
    // B
    pwm_p_data.pwm_aligned_mode = pwm_edge_aligned;         //边沿对齐
    pwm_p_data.pwm_ch_num = pwm_ch2;                        //通道号
    pwm_p_data.frequency = 1000;                            //1KHz
    pwm_p_data.duty = 0;                                    //上电输出0%占空比
    pwm_p_data.h_pin = IO_PORTB_05;                         //任意引脚
    pwm_p_data.l_pin = -1;                                  //任意引脚,不需要就填-1
    pwm_p_data.complementary_en = 0;                        //两个引脚的波形, 0: 同步,  1: 互补，互补波形的占空比体现在H引脚上
    mcpwm_init(&pwm_p_data);
   

}


void fc_driver(u8 r,u8 g ,u8 b)
{
    u32 duty1, duty2, duty3;
    duty1 = r * 10000 / 255;      //占空比转为0~10000
    duty2 = g * 10000 / 255;      //占空比转为0~10000
    duty3 = b * 10000 / 255;      //占空比转为0~010000


    // if (fc_effect.rgbw_on_off_flag == DEVICE_OFF)
    // {
    //     mcpwm_set_duty(pwm_ch0, 0);
    //     mcpwm_set_duty(pwm_ch1, 0);
    //     mcpwm_set_duty(pwm_ch2, 0);
    //     mcpwm_set_duty(pwm_ch3, 0);
    // }
    // else
    {
        mcpwm_set_duty(pwm_ch0, duty2); //r  故意将duty2和duty1互换，因为初始化使用GRB，幻彩与结构体的rgb适配
        mcpwm_set_duty(pwm_ch1, duty1); //g
        mcpwm_set_duty(pwm_ch2, duty3); //b
    }
}







