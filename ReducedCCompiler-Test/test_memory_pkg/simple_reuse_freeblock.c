int main()
{
    int prev_addr;
    for (int i = 0; i < 6; i = i + 1)
    {
        int ptr = malloc(10);
        if (i > 0)
        {
            printn(ptr == prev_addr);
            putchar(10);
        }
        prev_addr = ptr;
        free(ptr);
    }
}
