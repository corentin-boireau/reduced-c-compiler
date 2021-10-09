int main()
{
	printn(0);
	putchar(10);
	printn(-5134656);
	putchar(10);
	printn(4354);
	putchar(10);
	
	int array;
	int i;
	array = malloc(10);
	for (i = 0; i < 10; i = i + 1)
	{
		array[i] = i;
		printn(array[i]);
		putchar(10);
	}
	free(array);
	
	return 0;
}
