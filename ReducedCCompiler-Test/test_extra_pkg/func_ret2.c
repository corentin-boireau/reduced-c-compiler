int add2(int a, int b) 
{
    return a + b;
}

int add(int a, int b)
{
    return add2(a,b);
}

int main()
{
    int a;
    a = -3;
    print add(14,a);
}

