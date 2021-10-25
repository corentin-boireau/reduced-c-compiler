int main() {
    int i,j,k,res;
    res = 0;
    for(i = 0; i < 5*2; i = i + 1)
    {
        for(j = 0; j < 5; j = j + 1)
        {
            for(k = 0; k < j; k = k + 1)
            {
                res = res + k;
            }
        }
    }

    print res;
}
