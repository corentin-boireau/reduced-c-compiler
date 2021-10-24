int main()
{
	printn(0);
	putchar(10);
	printn(-5134656);
	putchar(10);
	printn(4354);
	putchar(10);
	
    int array_array = malloc(10);
    int i;
    int j;
    for (i = 0; i < 10; i = i + 1)
    {
        array_array[i] = malloc(10);
        for (j = 0; j < 10; j = j + 1)
        {
            array_array[i][j] = i * 10 + j;
        }
    }
    for (i = 0; i < 10; i = i + 1)
    {
        for (j = 0; j < 10; j = j + 1)
        {
            printn(array_array[i][j]);
            putchar(9);
        }
        putchar(10);
        free(array_array[i]);
    }

    memdump(*0 + 1, *0 + 150, 12);
    free(array_array);
	
	return 0;
}
