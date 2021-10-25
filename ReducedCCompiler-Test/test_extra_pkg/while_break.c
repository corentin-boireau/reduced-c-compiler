int main() {
    int i, res;
    i = res = 0;
    while(i < 100)
    {
        i = i + 1;
        res = res + 1;

        if (i == 12)
            break;
    }
    print res;
}
