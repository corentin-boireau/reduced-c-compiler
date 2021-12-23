int b;

int printb()
{
    print b;
}
int main()
{
    int a = &b;
    *a = 34;
    printb();
}
