int main() 
{
    int a;
    a = 0;
    while (a < 5)
    {
        print 1000;
        a = a + 1;
        if (a > 3)
            continue;
        print a;
    }

    a = 0;
    do
    {
        print 1000;
        a = a + 1;
        if (a > 3)
            continue;
        print a;
    } while (a < 5);

    for (a = 0; a < 5; a = a + 1)
    {
        print 1000;
        if (a > 3)
            continue;
        print a;
    }
}
