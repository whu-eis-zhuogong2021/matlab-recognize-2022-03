/*
 * keyboard.c
 *
 *  Created on: 2021年12月25日
 *      Author: IvanZheng
 */
#include "msp430.h"
#include "keyboard.h"
unsigned  char KeyVal;
void init_key(void)
{
    //先设置输入输出
    //行为输出 P1.5 P2.4 P2.5 P4.3
    P1DIR |= BIT5;
    P2DIR |= (BIT4+BIT5);
    P4DIR |= BIT3;
    //列为输入 P2.0 P1.2 P1.3 P1.4
    P2DIR &= ~BIT0;
    P1DIR &= ~(BIT2+BIT3+BIT4);
    //输入设置上拉，输出特征设置为高电平 按键按下就会出低电平 不按为高电平
    P2REN |=BIT0;
    P1REN |= (BIT2+BIT3+BIT4);
    P2OUT |=BIT0;
    P1OUT|=(BIT2+BIT3+BIT4);
}
unsigned short int key()
{
    unsigned char ReadData=0x00;
    //扫描第一行,第一行输出低电平，第二三四行输出高电平
    P1OUT |= BIT5;
    P2OUT |= (BIT4+BIT5);
    P4OUT |= BIT3;
    P1OUT &= ~BIT5;
    //只看输入位
    ReadData= ((P2IN | ~BIT0) & (P1IN | ~(BIT2+BIT3+BIT4)))^0xff;
    switch(ReadData)
    {
    //第一列
    case 0x01:
    KeyVal=1;
    _delay_cycles(300000);
    return KeyVal;
    //第二列
    case 0x04:
    KeyVal=2;
    _delay_cycles(300000);
    return KeyVal;
    //第三列
    case 0x08:
    KeyVal=3;
    _delay_cycles(300000);
    return KeyVal;
    //第四列
    case 0x10:
    KeyVal=4;
    _delay_cycles(300000);
    return KeyVal;
    default:
    KeyVal=0;
    break;
   }
    ReadData=0x00;
    P1OUT |= BIT5;
    P2OUT |= (BIT4+BIT5);
    P4OUT |= BIT3;
    //扫描第二行,第二行输出低电平，第一三四行输出高电平
    P2OUT &=~BIT4;
    //只看输入位
    ReadData= ((P2IN | ~BIT0) & (P1IN | ~(BIT2+BIT3+BIT4)))^0xff;
    switch(ReadData)
    {
    //第一列
    case 0x01:
    KeyVal=5;
    _delay_cycles(300000);
    return KeyVal;
    //第二列
    case 0x04:
    KeyVal=6;
    _delay_cycles(300000);
    return KeyVal;
    //第三列
    case 0x08:
    KeyVal=7;
    _delay_cycles(300000);
    return KeyVal;
    //第四列
    case 0x10:
    KeyVal=8;
    _delay_cycles(300000);
    return KeyVal;
    default:
    KeyVal=0;
    break;
    }
    ReadData=0x00;
    P1OUT |= BIT5;
    P2OUT |= (BIT4+BIT5);
    P4OUT |= BIT3;
    //扫描第三行,第三行输出低电平，第一二四行输出高电平
    P2OUT &=~BIT5;
    //只看输入位
    ReadData= ((P2IN | ~BIT0) & (P1IN | ~(BIT2+BIT3+BIT4)))^0xff;
    switch(ReadData)
    {
    //第一列
    case 0x01:
    KeyVal=9;
    _delay_cycles(300000);
    return KeyVal;
    //第二列
    case 0x04:
    KeyVal=10;
    _delay_cycles(300000);
    return KeyVal;
    //第三列
    case 0x08:
    KeyVal=11;
    _delay_cycles(300000);
    return KeyVal;
    //第四列
    case 0x10:
    KeyVal=12;
    _delay_cycles(300000);
    return KeyVal;
    default:
    KeyVal=0;
    break;
    }
    ReadData=0x00;
    P1OUT |= BIT5;
    P2OUT |= (BIT4+BIT5);
    P4OUT |= BIT3;
    //扫描第四行,第四行输出低电平，第一二三行输出高电平
    P4OUT &= ~BIT3;
    //只看输入位
    ReadData= ((P2IN | ~BIT0) & (P1IN | ~(BIT2+BIT3+BIT4)))^0xff;
    switch(ReadData)
    {
    //第一列
    case 0x01:
    KeyVal=13;
    _delay_cycles(300000);
     return KeyVal;
    //第二列
    case 0x04:
    KeyVal=14;
    _delay_cycles(300000);
    return KeyVal;
    //第三列
    case 0x08:
    KeyVal=15;
    _delay_cycles(300000);
    return KeyVal;
    //第四列
    case 0x10:
    KeyVal=16;
    _delay_cycles(300000);
    return KeyVal;
    default:
    KeyVal=0;
    break;
    }


    return KeyVal;

}




