int main() 
{
	int a;
	int b, c;

	print a = 4;
	b = 434;
	c = 23;
	print b;
	print c;

	a = b = c = 3 + 23;

	print a;
	print b;
	print c;
	{
		int a;
		{
			a = 89;
			int b;
			b = 34;
		}
		print a;
		print b;
		b = 32;
	}
	print a;
	print b;
}
