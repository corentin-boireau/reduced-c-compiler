int main() {
    int i,j,res,k;
    i = j = res = k = 0;

    while(j<3)
    {
        k = 0;
        while(k<2)
        {
            i = 0;
            while(i<10)
            {
                i = i + 1;
                res = res + 1;
            }
            k = k +1;
        }
        j = j + 1;
    }
    print res;
}
