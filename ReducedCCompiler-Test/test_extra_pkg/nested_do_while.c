int main() {
    int i,j,res,k;
    i = j = res = k = 0;

    do
    {
        k = 0;
        do
        {
            i = 0;
            do
            {
                i = i + 1;
                res = res + 1;
            } while(i<10);
            k = k +1;
        } while(k<2);
        j = j + 1;
    } while(j<3);
    print res;
}
