int main() {
    int i,res;
    i=res=0;
    do
    {
        i = i + 1;
        if (i % 2 == 0)
            continue;
        res = res + 1;
    } while(i<100);
    print res;
}
