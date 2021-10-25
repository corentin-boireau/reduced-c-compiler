int fact(int a)
{
    if (a < 2)
        return a;
    else
        return a * fact(a-1);
}
int main()
{
    print fact(16);
}
