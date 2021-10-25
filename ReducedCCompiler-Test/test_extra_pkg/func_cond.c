/*int printf(int a)
{
    print a;
}*/

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
        printf(4);
    }else
    {
        if(check(a,20))
        {
            printf(16);
        }
        else
        {
            printf(-3);
        }
        printf (333);
    }
}
