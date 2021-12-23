int main()
{
    int a = 3;
    int b = &*a;
    print b;
    int c = 3000; // Arbitrary address to avoid using the runtime
    int c_third = &c[2];
    c[2] = 4;
    print *c_third;
}
