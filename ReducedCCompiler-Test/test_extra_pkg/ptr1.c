int main()
{
    int a = 0, b = 0;
    b = &a;
    *b = 10;
    printf(a);
}
