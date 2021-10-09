int printn(int n)
{
    if (n < 0)
    {
        putchar(45);
        printn(-n);
    }
    else
    {
        if (n / 10 != 0)
            printn(n / 10);
        putchar(48 + n % 10);
    }
}

int malloc(int size)
{
    int ptr;
    ptr = *0;
    *0 = ptr + size;

    return ptr;
}

int free(int ptr)
{
    return 0;
}
