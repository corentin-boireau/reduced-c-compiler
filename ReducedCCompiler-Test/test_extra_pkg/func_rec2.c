int fibo(int a)
{
    if (a < 2)
        return a;
    else
        return fibo(a-2) + fibo(a-1);
}
int main()
{
    print fibo(34);
}
