int main()
{
	int ptr_a;
	ptr_a = 0;
	print *ptr_a;
	print ptr_a[2];
	int i;
	for (i = 0; i < 10; i = i + 1)
	{
		print *(ptr_a + i);
	}

	return 0;
}
