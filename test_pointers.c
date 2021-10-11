int main()
{
	putchar(10);
	int ptr_a;
	ptr_a = 0;
	print *ptr_a;
	print ptr_a[2];
	int i;
	for (i = 0; i < 10; i = i + 1)
	{
		print i;
		print *(ptr_a + i);
	}

	print *(ptr_a + 2);
	print *(ptr_a + 3);
	int ptr_b;
	ptr_b = 0;
	*ptr_b = 64;
	print  *ptr_a;
	
	return 0;
}
