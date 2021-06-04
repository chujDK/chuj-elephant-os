int _start()
{
    int i = 0;
    while(1)
    {
        i++;
        __asm__ volatile(
            "movb $\'K\',%gs:6"
           );
    };
    return 0;
}

