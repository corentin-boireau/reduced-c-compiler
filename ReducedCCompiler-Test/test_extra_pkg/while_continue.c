int main() {
    int i,res;
    i=res=0;
    while(i<100)
    {
        i = i + 1;
        if (i>12)
            continue;
        res = res + 1;
    }
    print res;
}
