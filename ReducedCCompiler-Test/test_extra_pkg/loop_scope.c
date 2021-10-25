int main() {
    int b,res;
    res = 5;
    b = 1;
    while(b)
    {
        int res;
        res = 3;
        b = 0;
    }

    for(b = 0; b < 4; b = b + 1)
    {
        res = res + b;
    }
    print res;
}
