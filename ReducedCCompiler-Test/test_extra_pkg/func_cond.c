int check(int a, int b)
{
    return a < b;
}

int main()
{
    int a;
    a = 19;
    if(check(a,10))
    {
        print 4;
    }else
    {
        if(check(a,20))
        {
            print 16;
        }
        else
        {
            print -3;
        }
        print 333;
    }
}
