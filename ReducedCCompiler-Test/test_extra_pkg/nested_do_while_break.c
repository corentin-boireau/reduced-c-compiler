int main() {
    int i,j,res;
    i=res=0;
    do
    {
        i = i + 1;
        j = 0;

        do
        {
            j = j + 1;
            res = res + 1;
            if (j == 12)
                break;
        } while (j < 30);

    } while(i < 4);
    print res;
}
