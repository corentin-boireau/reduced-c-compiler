int main() {
    int i, res;
    i = res = 0;
    do
    {
        i = i + 1;
        res = res + 1;

        if (i == 12)
            break;
    } while(i < 100);
    print res;
}
