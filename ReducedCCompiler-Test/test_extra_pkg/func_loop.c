int inc(int a)
{
    return a + 1;
}

int compute(int a, int b)
{
    return a * b;

}

int check(int a, int b)
{
    return a < b;
}

int main()
{
    int res,i;
    res = 1;
    for(i = 1; check(i, 5); i=inc(i))
    {
        res = res + compute(res,i);
    }

    i = 1;
    while(check(i, 5))
    {
        res = res + compute(res,i);
        i=inc(i);
    }

    i = 1;
    do
    {
        res = res + compute(res,i);
        i=inc(i);
    }while(check(i,5));
    print res;
}
