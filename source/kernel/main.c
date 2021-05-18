int _start()
{
    int i = 0;
    while(1)
    {
        i++;
        asm volatile(
            "movb $\'K\',%gs:6"
           );
    };
    return 0;
}

