int main()
{
    int a = 3;
    int b = &*a;
    print b; 
    int c = malloc(3);
    int c_third = &c[2];
    c[2] = 4;
    print *c_third;
    free(c);
}
