//宏定义
#define CALADC12_15V_30C  *((unsigned int *)0x1A1A)   // Temperature Sensor Calibration-30 C
//See device datasheet for TLV table memory mapping
#define CALADC12_15V_85C  *((unsigned int *)0x1A1C)   // Temperature Sensor Calibration-85 C
//头文件声明
#include <msp430.h> 
#include "keyboard.h"
#include "/oled/oled.h"
#include <stdio.h>
//函数声明
void timerA_init();
void run_time();
void show_time();
void run_date();
void show_date();
void run_year();
void get_key_value();
void mode();
void display();
void set_time();
void run_week();
void show_week();
void stopwatch();
void show_stopwatch();
void set_date();
void run_date_year();
void set_time_12();
void show_time_12();
void temp_init();
void temp_AD();
void show_temp();
void alarm_change();
void set_alarm();
void write_flash_int(unsigned int addr, int *array, int count);
void read_flash_int1(unsigned int addr, int *array, int count);
void store();
void read();

//全局变量声明
int hour = 9, minute = 59, second = 50, h_12; //初始时，分，秒
int year = 2022, month = 2, day = 14, week = 0; //出生日期为该天
int am_or_pm = 0;
int flash_data[6];
//闹钟存储时钟，5个闹钟
//分别存储 hour,minute,on/off(时间分开存到前两个里）
int alarms[5][5] = { 0 }; //
int alarms_judge[5][2] = { 0 };
int off = 0;
//温度

unsigned int temp;
volatile float temperatureDegC;
volatile float temperatureDegF;
int temtemp = 0;
//状态标志：0：走时；1：时间设置；2：日期设置；3：闹钟设置；
int state = 0, state1 = 0;
//12/24小时制模式切换,1：表示24小时，0：表示12小时
int t24or12 = 0;
int keykey = 0;
int ms = 0, s = 0, m = 0;
int value = 0; //从键盘扫描到的值
int key_value = 0; //真正的按键值
int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;

    timerA_init(); //时钟，计数器初始化
    OLED_Init(); //初始化屏幕
    init_key(); //初始化按键
    temp_init();
    read_flash_int1(0x001800, flash_data, 6); //读取出每位数字
    read(); //存储时间
    //int key_value = 0;
    while (1)
    {
        temp_AD();

        if (state == 10)
            set_alarm();
        else
            ;
    }

}
//时钟，计数器初始化
void timerA_init()
{

    TA0CTL |= TASSEL_1 + ID__8 + TACLR + MC_1; // 8分频，ACLK, 清除计数器，增计数模式 32768/8
    TA0CCR0 = 4096;
    TA1CTL |= TASSEL_1 + ID__8 + TACLR + MC_1;
    TA1CCR0 = 410;

    TA0CCTL0 |= CCIE; //使能定时器中断（CCR0单源中断）
    TA1CCTL0 |= CCIE;
    P3DIR |= BIT6; //1s中断的指示灯
    P3OUT &= ~BIT6;
    P3DIR |= BIT5; //10hz中断的指示灯
    P3OUT &= ~BIT5;
    P8DIR |= BIT2;
    P8OUT &= ~BIT2;
    P8DIR ^= BIT2;
    P1DIR|=BIT2;
//    P1OUT&=~BIT2;
    _EINT(); //使能总中断
}
//TA0CCR0中断服务函数,计时+1
#pragma vector = TIMER0_A0_VECTOR
__interrupt void TIMER0_A0_ISR(void)
{
    run_time();
    P3OUT ^= BIT6;


//    if (state == 5) //调日期时显示时间和温度
//    {
//        show_time();
//        show_temp();
//    }
    if (state == 5)
    {
        OLED_Clear();
        set_date();
    }
    if (state == 0)
    {
        if (t24or12 == 0)
        {
            display();
            OLED_ShowString(96, 4, "  ");
            show_temp();
        }
        else if (t24or12 == 1)
        {
            show_time_12();
            run_date_year();
            show_date();
            show_week();

            show_temp();
        }
    }
    else
        ;
    store(); //时间转化为每位数字
    write_flash_int(0x001800, flash_data, 6); //存进去
}

//TA1CCR0中断服务函数，用来扫描按键
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
    P3OUT ^= BIT5;

//    if (state == 0)
//    {
//        get_key_value();
//    }
    get_key_value(); //获取按键值以及得到状态标志的值
    mode();
    if (state == 2) //进入
    {
        get_key_value();
        if (state1 == 1) //清零->数字C
        {
            m = 0, s = 0, ms = 0;
            show_stopwatch();
        }
        else if (state1 == 2) //停止->数字2
        {
            show_stopwatch();
        }
        else if (state1 == 3) //开始计时->数字1
        {
            if (ms == 90)
            {
                ms += 9;
                show_stopwatch();
            }
            else
                ms += 10;
            if (ms == 99)
            {
                ms = 0;
                s++;
                if (s == 60)
                {
                    s = 0;
                    m++;
                }
            }
            show_stopwatch();
        }
    }
    if (state == 6)
        set_alarm();
}
#pragma vector=ADC12_VECTOR
__interrupt void ADC12ISR(void)
{
    switch (__even_in_range(ADC12IV, 34))
    {
    case 0:
        break;                           // Vector  0:  No interrupt
    case 2:
        break;                           // Vector  2:  ADC overflow
    case 4:
        break;                           // Vector  4:  ADC timing overflow
    case 6:                                  // Vector  6:  ADC12IFG0
        temp = ADC12MEM0;                       // 读取结果，中断标志已被清除
        __bic_SR_register_on_exit(LPM4_bits);   // Exit active CPU
    case 8:
        break;                           // Vector  8:  ADC12IFG1
    case 10:
        break;                           // Vector 10:  ADC12IFG2
    case 12:
        break;                           // Vector 12:  ADC12IFG3
    case 14:
        break;                           // Vector 14:  ADC12IFG4
    case 16:
        break;                           // Vector 16:  ADC12IFG5
    case 18:
        break;                           // Vector 18:  ADC12IFG6
    case 20:
        break;                           // Vector 20:  ADC12IFG7
    case 22:
        break;                           // Vector 22:  ADC12IFG8
    case 24:
        break;                           // Vector 24:  ADC12IFG9
    case 26:
        break;                           // Vector 26:  ADC12IFG10
    case 28:
        break;                           // Vector 28:  ADC12IFG11
    case 30:
        break;                           // Vector 30:  ADC12IFG12
    case 32:
        break;                           // Vector 32:  ADC12IFG13
    case 34:
        break;                           // Vector 34:  ADC12IFG14
    default:
        break;
    }
}
void temp_init()
{
    REFCTL0 &= ~REFMSTR;                // Reset REFMSTR to hand over control to
                                        // ADC12_A ref control registers
    ADC12CTL0 = ADC12SHT0_8 + ADC12REFON + ADC12ON;
    // 设置参考电压为1.5V，打开AD
    ADC12CTL1 = ADC12SHP;                     // 采样保持脉冲设置为内部定时器
    ADC12MCTL0 = ADC12SREF_1 + ADC12INCH_10;  // AD的A10通道连接至温度传感器输出
    ADC12IE = 0x001;                         // ADC_IFG upon conv result-ADCMEMO
    __delay_cycles(100);               // Allow ~100us (at default UCS settings)
                                       // for REF to settle
    ADC12CTL0 |= ADC12ENC;                    // 使能AD

}
void temp_AD()
{
    ADC12CTL0 &= ~ADC12SC;
    ADC12CTL0 |= ADC12SC;                   // 开始采样

    __bis_SR_register(LPM4_bits + GIE);     // LPM0 with interrupts enabled
    __no_operation();

    // Temperature in Celsius. See the Device Descriptor Table section in the
    // System Resets, Interrupts, and Operating Modes, System Control Module
    // chapter in the device user's guide for background information on the
    // used formula.
    temperatureDegC = (float) (((long) temp - CALADC12_15V_30C) * (85 - 30))
            / (CALADC12_15V_85C - CALADC12_15V_30C) + 30.0f; //摄氏度换算

    temtemp = temperatureDegC * 100;

    // Temperature in Fahrenheit Tf = (9/5)*Tc + 32
    temperatureDegF = temperatureDegC * 9.0f / 5.0f + 32.0f;    //华氏度换算
    _delay_cycles(1000000);
    __no_operation();                       // SET BREAKPOINT HERE
}
void show_temp()
{
    OLED_ShowString(1, 6, "Temp=");
    OLED_ShowString(57, 6, ".");
    OLED_ShowNum(41, 6, temtemp / 100, 2, 16);
    OLED_ShowNum(65, 6, temtemp % 100, 2, 16);    //温度显示
}
//时间运行函数，需要全局变量hour,minute,second
void run_time()
{
    int i = 0;
    second++;
    if (second == 60)
    {
        minute++;
        second = 0;
        if (minute == 60)
        {
            hour++;
            minute = 0;
            if (hour > 12)
                am_or_pm = 1;
            else
                am_or_pm = 0;
            if (hour == 24)
            {
                hour = 0;
            }
        }
    }
    for (i = 0; i < 5; i++)    //判断闹钟是否关闭
    {
        if (hour == alarms_judge[i][0] && minute == alarms_judge[i][1]
                && alarms[i][4] == 1)
        {
            P8DIR ^= BIT2;
            off = i;
        }
        if (alarms[off][4] == 3)
            P8OUT &= ~ BIT2;

    }
}

//显示时间的函数
void show_time()
{

    if (hour < 10)
    {
        OLED_ShowNum(0, 4, 0, 1, 16); //显示小时
        OLED_ShowNum(8, 4, hour, 1, 16); //显示小时
    }
    else
        OLED_ShowNum(0, 4, hour, 2, 16); //显示小时
    OLED_ShowChar(20, 4, 58); //显示冒号
    if (minute < 10)
    {
        OLED_ShowNum(32, 4, 0, 1, 16); //显示分钟
        OLED_ShowNum(40, 4, minute, 1, 16); //显示分钟
    }
    else
        OLED_ShowNum(32, 4, minute, 2, 16); //显示分钟
    OLED_ShowChar(52, 4, 58); //显示冒号
    if (second < 10)
    {
        OLED_ShowNum(64, 4, 0, 1, 16); //显示秒
        OLED_ShowNum(72, 4, second, 1, 16); //显示秒
    }
    else
        OLED_ShowNum(64, 4, second, 2, 16); //显示秒
}
void show_time_12()
{
    if (hour >= 12)
    {
        am_or_pm = 1;
    }
    else
    {
        am_or_pm = 0;
    }
    if (hour >= 13)
    {
        h_12 = hour - 12;

    }
    else
    {
        h_12 = hour;
    }
    if (h_12 < 10)
    {
        OLED_ShowNum(0, 4, 0, 1, 16); //显示小时
        OLED_ShowNum(8, 4, h_12, 1, 16); //显示小时
    }
    else
        OLED_ShowNum(0, 4, h_12, 2, 16); //显示小时
    OLED_ShowChar(20, 4, 58); //显示冒号
    if (minute < 10)
    {
        OLED_ShowNum(32, 4, 0, 1, 16); //显示分钟
        OLED_ShowNum(40, 4, minute, 1, 16); //显示分钟
    }
    else
        OLED_ShowNum(32, 4, minute, 2, 16); //显示分钟
    OLED_ShowChar(52, 4, 58); //显示冒号
    if (second < 10)
    {
        OLED_ShowNum(64, 4, 0, 1, 16); //显示秒
        OLED_ShowNum(72, 4, second, 1, 16); //显示秒
    }
    else
        OLED_ShowNum(64, 4, second, 2, 16); //显示秒
    if (am_or_pm == 0)
        OLED_ShowString(96, 4, "AM");
    else
        OLED_ShowString(96, 4, "PM");
}

//日期运行函数,需要全局变量，year，month，day
void run_date()
{
    int is_leap_year = 0;
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) //判断是否为闰年，是的话为1，不是为0
    {
        is_leap_year = 1;
    }
    else
    {
        is_leap_year = 0;
    }

    day++; //增加天数
    //增加月份，天数归1
    if (day == 32)    //当该月份为大月时
    {
        day = 1;
        month++;
    }
    else if ((month == 4 || month == 6 || month == 9 || month == 11)
            && day == 31)    //当该月为小月时
    {
        day = 1;
        month++;
    }
    else if (month == 2)    //判断二月的两种特殊情况
    {
        if (!is_leap_year && day == 29)
        {
            day = 1;
            month++;
        }
        else if (is_leap_year && day == 30)
        {
            day = 1;
            month++;
        }
    }
}
//日期显示函数
void show_date()
{
    OLED_ShowNum(0, 1, year, 4, 16);    //显示年
    OLED_ShowChar(32, 1, 46);    //显示英文句号
    if (month < 10)
    {
        OLED_ShowNum(40, 1, 0, 1, 16);
        OLED_ShowNum(48, 1, month, 1, 16);    //显示月
    }
    else
        OLED_ShowNum(40, 1, month, 2, 16);
    OLED_ShowChar(58, 1, 46);    //显示英文句号
    if (day < 10)
    {
        OLED_ShowNum(64, 1, 0, 1, 16);
        OLED_ShowNum(72, 1, day, 1, 16);
    }
    else
        OLED_ShowNum(64, 1, day, 2, 16);    //显示日

}
//年份运行函数
void run_year()
{
    if (month == 13 && day == 1)
    {
        year++;
        month = 1;
        day = 1;
    }
}
//获取按键值
void get_key_value()
{
    value = key();
    switch (value)
    {
    case 1:
        key_value = 1;
        t24or12 = 0;
        break;
    case 2:
        key_value = 2;

        break;
    case 3:
        key_value = 3;
        state = 6;
        break;
    case 4:
        key_value = 10;
        state = 0;
        break;
    case 5:
        key_value = 4;
        break;
    case 6:
        key_value = 5;
        state1 = 2;
        break;
    case 7:
        key_value = 6;
        state1 = 3;
        break;
    case 8:
        key_value = 11;
        state = 1;
        break;
    case 9:
        key_value = 7;
        alarms[off][4] = 3;
        break;
    case 10:
        key_value = 8;
        t24or12 = 1;
        break;
    case 11:
        key_value = 9;
        break;
    case 12:
        key_value = 12;
        state = 2;
        state1 = 1;
        break;
    case 13:
        key_value = 15;
        state = 4;
       break;
    case 14:    //此处有问题，有瑕疵
        key_value = 14;
        break;
    case 15:    //此处有问题，有瑕疵
        key_value = 16;
        state = 5;
        break;
    case 16:
        key_value = 13;
        state = 3;
        break;
    default:
        break;
    }

}
//模式选择，默认为走时模式
//状态标志：0：走时；1：时间设置；2：日期设置；3：闹钟设置；
void mode()
{

    switch (state)
    {
    case 0:
        break;
    case 1:
        if (t24or12 == 0)
        {
            set_time();
            OLED_ShowString(96, 4, "  ");
        }
        else
            set_time_12();
        break;
    case 2:
        break;
    default:
        break;
    }
}
//显示时间函数
void display()
{
    show_time(); //显示时间
    show_week();
    run_date_year();
    show_date(); //显示日期
    show_week(); //显示星期
}
void run_date_year()
{
    if (hour == 0 && minute == 0 && second == 0)
    {
        run_date();
        run_year();
        show_date(); //显示日期
        _delay_cycles(1000000);
    }
}
//设置时间函数
void set_time()
{

    int i, single[6] = { 10, 10, 10, 10, 10, 10 };
    unsigned char location;

    OLED_ShowChar(20, 4, 58); //显示冒号
    OLED_ShowChar(52, 4, 58); //显示冒号
    for (i = 0; i < 6; i++)
    {

        switch (i)
        {
        case 0:
            location = 0;
            break;
        case 1:
            location = 8;
            break;
        case 2:
            location = 32;
            break;
        case 3:
            location = 40;
            break;
        case 4:
            location = 64;
            break;
        case 5:
            location = 72;
            break;
        }
        while (1)
        {
            key_value = 0;
            key_value = key();
            if (key_value != 0)
            {
                switch (key_value)
                {
                case 1:
                    OLED_ShowChar(location, 4, '1');
                    single[i] = 1;
                    break;
                case 2:
                    OLED_ShowChar(location, 4, '2');
                    single[i] = 2;
                    break;
                case 3:
                    OLED_ShowChar(location, 4, '3');
                    single[i] = 3;
                    break;
                case 5:
                    OLED_ShowChar(location, 4, '4');
                    single[i] = 4;
                    break;
                case 6:
                    OLED_ShowChar(location, 4, '5');
                    single[i] = 5;
                    break;
                case 7:
                    OLED_ShowChar(location, 4, '6');
                    single[i] = 6;
                    break;
                case 9:
                    OLED_ShowChar(location, 4, '7');
                    single[i] = 7;
                    break;
                case 10:
                    OLED_ShowChar(location, 4, '8');
                    single[i] = 8;
                    break;
                case 11:
                    OLED_ShowChar(location, 4, '9');
                    single[i] = 9;
                    break;
                case 14:
                    OLED_ShowChar(location, 4, '0');
                    single[i] = 0;
                    break;

                default:
                    break;
                }
                if (key_value != 8 && single[i] != 10)
                    break;
            }
        }
        int sure = 0;
        int flag = 0;

        //输入是否正确的标志
        if (i == 5) //当输入到第六位数时，开始检测是否正常
        {

            while (1)
            {
                sure = key();
                if (sure != 0)
                    break;
            }
            if ((single[0] * 10 + single[1]) > 23
                    || (single[2] * 10 + single[3]) > 60
                    || (single[4] * 10 + single[5]) > 60)
            {
                flag = 0;
            }
            else
            {
                flag = 1;
            }

            switch (flag)
            {
            case 0: //当输入的时间不规范时
                OLED_Clear(); //清屏
                OLED_ShowCHinese(8, 4, 0); //输
                OLED_ShowCHinese(24, 4, 1); //入
                OLED_ShowCHinese(40, 4, 2); //错
                OLED_ShowCHinese(54, 4, 3); //误
                _delay_cycles(500000);
                OLED_Clear(); //清屏
                state = 0;
                break;
            case 1: //当正确输入时间时
                if (single[0] == 0)
                    hour = single[1];
                else
                    hour = single[0] * 10 + single[1];
                if (single[2] == 0)
                    minute = single[3];
                else
                    minute = single[2] * 10 + single[3];
                if (single[4] == 0)
                    second = single[4];
                else
                    second = single[4] * 10 + single[5];
                if (second > 30 && second <= 60)
                {
                    second = 0;
                    minute++;
                    if (minute == 60)
                    {
                        hour++;
                        minute = 0;
                        if (hour == 24)
                            hour = 0;
                        else
                            ;
                        run_date_year();
                    }
                }
                else
                {
                    second = 0;
                }
                state = 0;
                break;
            default:
                break;
            }
        }
        else
        {

        }

    }
}

//秒表显示函数
void show_stopwatch()
{

    if (m < 10) //显示分钟
    {
        OLED_ShowNum(0, 4, 0, 1, 16);
        OLED_ShowNum(8, 4, m, 1, 16);
    }
    else
        OLED_ShowNum(0, 4, m, 2, 16);
    OLED_ShowChar(20, 4, 58); //显示冒号
    if (s < 10) //显示秒
    {
        OLED_ShowNum(32, 4, 0, 1, 16);
        OLED_ShowNum(40, 4, s, 1, 16);
    }
    else
        OLED_ShowNum(32, 4, s, 2, 16);
    OLED_ShowChar(52, 4, 58); //显示冒号
    if (ms < 10) //显示毫秒
    {
        OLED_ShowNum(64, 4, 0, 1, 16);
        OLED_ShowNum(72, 4, ms, 1, 16);
    }
    else
        OLED_ShowNum(64, 4, ms, 2, 16);
}
//计算星期的函数
void run_week()
{
    int c, y, m2, w;
    if (month == 1 || month == 2)
    {
        m2 = month + 12;
        y = year - 1;
    }
    else
    {
        m2 = month;
        y = year;
    }
    c = y / 100;
    y = y % 100;
    w = c / 4 - 2 * c + y + (13 * (m2 + 1)) / 5 + day - 1 + y / 4;
    week = w % 7;

}
void show_week()
{
    run_week();
    switch (week)
    {
    case 0:
        OLED_ShowString(97, 1, "Sun");
        break;
    case 1:
        OLED_ShowString(97, 1, "Mon");
        break;
    case 2:
        OLED_ShowString(97, 1, "Tue");
        break;
    case 3:
        OLED_ShowString(97, 1, "Wed");
        break;
    case 4:
        OLED_ShowString(97, 1, "Thu");
        break;
    case 5:
        OLED_ShowString(97, 1, "Fri");
        break;
    case 6:
        OLED_ShowString(97, 1, "Sat");
        break;
    default:
        break;
    }
}
void set_date()
{

    int i, single[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    unsigned char location;
    init_key();

    OLED_ShowChar(32, 1, 46); //显示冒号
    OLED_ShowChar(58, 1, 46); //显示冒号
    for (i = 0; i < 8; i++)
    {

        switch (i)
        {
        case 0:
            location = 0;
            break;
        case 1:
            location = 8;
            break;
        case 2:
            location = 16;
            break;
        case 3:
            location = 24;
            break;
        case 4:
            location = 40;
            break;
        case 5:
            location = 48;
            break;
        case 6:
            location = 64;
            break;
        case 7:
            location = 72;
            break;
        default:
            break;

        }
        while (1)
        {
            key_value = 0;
            key_value = key();
            if (key_value != 0)
            {

                switch (key_value)
                {
                case 1:
                    OLED_ShowChar(location, 1, '1');
                    single[i] = 1;
                    break;
                case 2:
                    OLED_ShowChar(location, 1, '2');
                    single[i] = 2;
                    break;
                case 3:
                    OLED_ShowChar(location, 1, '3');
                    single[i] = 3;
                    break;
                case 5:
                    OLED_ShowChar(location, 1, '4');
                    single[i] = 4;
                    break;
                case 6:
                    OLED_ShowChar(location, 1, '5');
                    single[i] = 5;
                    break;
                case 7:
                    OLED_ShowChar(location, 1, '6');
                    single[i] = 6;
                    break;
                case 9:
                    OLED_ShowChar(location, 1, '7');
                    single[i] = 7;
                    break;
                case 10:
                    OLED_ShowChar(location, 1, '8');
                    single[i] = 8;
                    break;
                case 11:
                    OLED_ShowChar(location, 1, '9');
                    single[i] = 9;
                    break;
                case 14:
                    OLED_ShowChar(location, 1, '0');
                    single[i] = 0;
                default:
                    break;
                }
                if (key_value != 16 && single[i] != 10)
                    break;
            }
        }

        int flag = 1; //输入是否正确的标志
        int ye;
        int spec = 0;
        if (i == 7) //当输入到第六位数时，开始检测是否正常
        {
            if (single[4] * 10 + single[5] > 12
                    || single[6] * 10 + single[7] > 31)
            {
                flag = 0;
            }
            else
            {
                flag = 1;
            }
            ye = single[0] * 1000 + single[1] * 100 + single[2] * 10
                    + single[3];
            if ((ye % 4 == 0 && ye % 100 != 0) || ye % 400 == 0)
                spec = 1;
            if (single[4] * 10 + single[5] == 2
                    && single[6] * 10 + single[7] == 29 && spec == 0)
                flag = 0;

            switch (flag)
            {
            case 0: //当输入的时间不规范时
                OLED_Clear(); //清屏
                OLED_ShowCHinese(8, 4, 0); //输
                OLED_ShowCHinese(24, 4, 1); //入
                OLED_ShowCHinese(40, 4, 2); //错
                OLED_ShowCHinese(54, 4, 3); //误
                _delay_cycles(1000000);
                OLED_Clear(); //清屏
                display();
                state = 0;
                break;
            case 1: //当正确输入时间时
                year = single[0] * 1000 + single[1] * 100 + single[2] * 10
                        + single[3];
                if (single[4] == 0)
                {
                    month = single[5];
                }
                else
                    month = single[4] * 10 + single[5];
                if (single[6] == 0)
                    day = single[7];
                else
                    day = single[6] * 10 + single[7];
                state = 0;
                break;
            default:
                break;
            }
        }
        else
        {

        }

    }
}
void set_time_12()
{

    init_key();
    int key_value = 0;
    int i, j, a[6] = { 10, 10, 10, 10, 10, 10 };
    unsigned char location;

    OLED_ShowChar(20, 4, 58); //显示冒号
    OLED_ShowChar(52, 4, 58); //显示冒号
    //OLED_ShowString(96, 4, "AM");

    if (am_or_pm == 0)
    {
        OLED_ShowString(96, 4, "AM");

    }
    else
    {
        OLED_ShowString(96, 4, "PM");

    }
    for (i = 0; i < 6 && j != 0; i++)
    {

        switch (i)
        {
        case 0:
            location = 0;
            break;
        case 1:
            location = 8;
            break;
        case 2:
            location = 32;
            break;
        case 3:
            location = 40;
            break;
        case 4:
            location = 64;
            break;
        case 5:
            location = 72;
            break;
        }
        while (1)
        {

            key_value = key();
            if (key_value != 0)
            {
                switch (key_value)
                {
                case 1:
                    OLED_ShowChar(location, 4, '1');
                    a[i] = 1;
                    key_value = 0;
                    break;
                case 2:
                    OLED_ShowChar(location, 4, '2');
                    a[i] = 2;
                    break;
                case 3:
                    OLED_ShowChar(location, 4, '3');
                    a[i] = 3;
                    break;
                case 4:
                    OLED_ShowChar(location, 4, 'A');
                    break;
                case 5:
                    OLED_ShowChar(location, 4, '4');
                    a[i] = 4;
                    break;
                case 6:
                    OLED_ShowChar(location, 4, '5');
                    a[i] = 5;
                    break;
                case 7:
                    OLED_ShowChar(location, 4, '6');
                    a[i] = 6;
                    break;
                case 8:
                    OLED_ShowChar(location, 4, 'B');
                    break;
                case 9:
                    OLED_ShowChar(location, 4, '7');
                    a[i] = 7;
                    break;
                case 10:
                    OLED_ShowChar(location, 4, '8');
                    a[i] = 8;
                    break;
                case 11:
                    OLED_ShowChar(location, 4, '9');
                    a[i] = 9;
                    break;
                case 12:
                    OLED_ShowChar(location, 4, 'C');
                    break;
                case 13:
                    break;

                case 14:
                    OLED_ShowChar(location, 4, '0');
                    a[i] = 0;
                    break;
                case 15:
                    break;
                case 16:
                    OLED_ShowChar(location, 4, 'D');
                    break;
                default:
                    break;
                }
                if (key_value != 16 && a[i] != 10)
                    break;
            }

        }

    }

    while (1)
    {
        key_value = key();
        if (key_value == 2)
        {
            if (am_or_pm == 0)
            {
//
                am_or_pm = 1;
            }
            else
            {
//
                am_or_pm = 0;
            }
        }
        if (am_or_pm == 0)
            OLED_ShowString(96, 4, "AM");
        else
            OLED_ShowString(96, 4, "PM");
        if (key_value == 15)
        {

            if (((a[0] * 10 + a[1]) > 12 || (a[2] * 10 + a[3]) > 60
                    || (a[4] * 10 + a[5]) > 60)
                    || ((am_or_pm == 0) && ((a[0] * 10 + a[1]) == 12)))
            {
                OLED_Clear(); /*clear OLED screen*/
                OLED_ShowString(0, 2, "error");
                _delay_cycles(800000);
                OLED_Clear(); /*clear OLED screen*/
                state = 0;
            }
            else
            {
                if (am_or_pm == 0)
                {
                    if (a[0] == 0)
                        hour = a[1];
                    else
                        hour = a[0] * 10 + a[1];
                }
                else
                {
                    if (a[0] == 0)
                        hour = a[1] + 12;
                    else
                        hour = a[0] * 10 + a[1] + 12;
                    if (hour == 24)
                        hour = 12;
                }
                if (a[2] == 0)
                    minute = a[3];
                else
                    minute = a[2] * 10 + a[3];
                if (a[4] == 0)
                    second = a[5];
                else
                    second = a[4] * 10 + a[5];
//
                state = 0;
            }
            break;
        }
    }

}

void set_alarm()
{

    init_key();
    int key_value = 0;
    OLED_Clear();
    int m = 1;
    unsigned char location;
    int i = 0, j;
    OLED_ShowString(2, 2, "alarm");
    OLED_ShowChar(42, 2, '1');
    OLED_ShowChar(20, 4, 58); //显示冒号
    if (alarms[0][4] == 0)
        OLED_ShowString(96, 4, "OFF");
    else
        OLED_ShowString(96, 4, "ON ");
    for (j = 0; j < 4 || m == 0; j++)
    {
        OLED_ShowNum(0, 4, alarms[i][0], 1, 16);
        OLED_ShowNum(8, 4, alarms[i][1], 1, 16);
        OLED_ShowNum(32, 4, alarms[i][2], 1, 16);
        OLED_ShowNum(40, 4, alarms[i][3], 1, 16);
        switch (j)
        {
        case 0:
            location = 0;
            break;
        case 1:
            location = 8;
            break;
        case 2:
            location = 32;
            break;
        case 3:
            location = 40;
            break;

        }
        while (1)
        {
            key_value = key();
            if (key_value != 0)
            {
                switch (key_value)
                {
                case 1:
                    OLED_ShowChar(location, 4, '1');
                    alarms[i][j] = 1;
                    key_value = 0;
                    break;
                case 2:
                    OLED_ShowChar(location, 4, '2');
                    alarms[i][j] = 2;
                    break;
                case 3:
                    OLED_ShowChar(location, 4, '3');
                    alarms[i][j] = 3;
                    break;
                case 4:
                    OLED_ShowChar(location, 4, 'A');
                    break;
                case 5:
                    OLED_ShowChar(location, 4, '4');
                    alarms[i][j] = 4;
                    break;
                case 6:
                    OLED_ShowChar(location, 4, '5');
                    alarms[i][j] = 5;
                    break;
                case 7:
                    OLED_ShowChar(location, 4, '6');
                    alarms[i][j] = 6;
                    break;
                case 8:
                    OLED_ShowChar(location, 4, 'B');
                    break;
                case 9:
                    OLED_ShowChar(location, 4, '7');
                    alarms[i][j] = 7;
                    break;
                case 10:
                    OLED_ShowChar(location, 4, '8');
                    alarms[i][j] = 8;
                    break;
                case 11:
                    OLED_ShowChar(location, 4, '9');
                    alarms[i][j] = 9;
                    break;
                case 12:
                    OLED_ShowChar(location, 4, 'C');
                    break;
                case 13: //切换第几个闹钟
                    i++;
                    j = -1;
                    OLED_Clear();
                    OLED_ShowChar(20, 4, 58); //显示冒号
                    OLED_ShowString(2, 2, "alarm");
                    if (i == 5)
                    {
                        i = 0;
                    }
                    break;
                case 14:
                    OLED_ShowChar(location, 4, '0');
                    alarms[i][j] = 0;
                    break;
                case 15:
                    if (alarms[i][4] == 0)
                        alarms[i][4] = 1;
                    else
                        alarms[i][4] = 0;
                    break;

                case 16:
                    m = 0;
                    state = 0;
                    break;
                default:
                    break;
                }
                switch (i)
                {
                case 0:
                    OLED_ShowChar(42, 2, '1');
                    break;
                case 1:
                    OLED_ShowChar(42, 2, '2');
                    break;
                case 2:
                    OLED_ShowChar(42, 2, '3');
                    break;
                case 3:
                    OLED_ShowChar(42, 2, '4');
                    break;
                case 4:
                    OLED_ShowChar(42, 2, '5');
                    break;
                }
                if (alarms[i][4] == 0)
                    OLED_ShowString(96, 4, "OFF");
                else if (alarms[i][4] == 1)
                    OLED_ShowString(96, 4, "ON ");
                if ((alarms[i][j] != 10 && j != 3) || (m == 0))
                    break;
                if (j == -1)
                    break;
            }

        }
        if (m == 0)
        {
            OLED_Clear();
            alarm_change();
            break;
        }

    }
}
void alarm_change()
{
    int i;
    for (i = 0; i < 5; i++)
    {
        alarms_judge[i][0] = alarms[i][0] * 10 + alarms[i][1];
        alarms_judge[i][1] = alarms[i][2] * 10 + alarms[i][3];
    }
}
//写入存储
void write_flash_int(unsigned int addr, int *array, int count)
{
    unsigned int *Flash_ptr;
    int i;
    Flash_ptr = (unsigned int*) addr;
    FCTL1 = FWKEY + ERASE;
    FCTL3 = FWKEY;
    *Flash_ptr = 0;
    FCTL1 = FWKEY + WRT;
    for (i = 0; i < count; i++)
    {
        *Flash_ptr++ = array[i];
    }
    FCTL1 = FWKEY;
    FCTL3 = FWKEY + LOCK;
}
//读取存储
void read_flash_int1(unsigned int addr, int *array, int count)
{
    int *address = (int*) addr;
    int i;
    for (i = 0; i < count; i++)
    {
        array[i] = *address++;
    }
}
//把时间转化进行存储
void store()
{
    flash_data[0] = hour;
    flash_data[1] = minute;
    flash_data[2] = second;
    flash_data[3] = year;
    flash_data[4] = month;
    flash_data[5] = day;
}
//把读取的存储转化
void read()
{
    hour = flash_data[0];
    minute = flash_data[1];
    second = flash_data[2];
    year = flash_data[3];
    month = flash_data[4];
    day = flash_data[5];
}
