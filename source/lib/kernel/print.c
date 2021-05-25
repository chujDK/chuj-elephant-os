#include "print.h"

int sys_putint(int num)
{
    char str_num[16];
    int cnt = 0, neg_flag = 0;
    if (num < 0)
    {
        num = -num;
        neg_flag = -1;
    }

    do
    {
        str_num[cnt++] = num % 10 + '0';
        num /= 10;
    }
    while (num);

    if (neg_flag == -1)
    {
        str_num[cnt++] = '-';
    }
    str_num[cnt] = 0;
    for (int i = 0; i < cnt/2; i++)
    {
        int t = str_num[i];
        str_num[i] = str_num[cnt - i - 1];
        str_num[cnt - i - 1] = t;
    }
    return sys_putstr(str_num);
}

int sys_puthex(unsigned int num)
{
    char str_num[12];
    int cnt = 0;
    while(num)
    {
        str_num[cnt] = num % 16;
        if (str_num[cnt] > 9)
        {
            str_num[cnt] += 'A' - 10;
        }
        else
        {
            str_num[cnt] += '0';
        }
        num /= 16;
        cnt++;
    }
    str_num[cnt] = 0;
    for (int i = 0; i < cnt/2; i++)
    {
        int t = str_num[i];
        str_num[i] = str_num[cnt - i - 1];
        str_num[cnt - i - 1] = t;
    }
    return sys_putstr(str_num);
}
