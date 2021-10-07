int main() 
{
    int a;
    a = 0;
    while (a < 5)
    {
        print 15;
        a = a + 1;
        if (a > 3)
            break;
        print a;
    }

    a = 0;
    do
    {
        print 15;
        a = a + 1;
        if (a > 3)
            break;
        print a;
    } while (a < 5);

    for (a = 0; a < 5; a = a + 1)
    {
        print 15;
        if (a > 3)
            break;
        print a;
    }
}
