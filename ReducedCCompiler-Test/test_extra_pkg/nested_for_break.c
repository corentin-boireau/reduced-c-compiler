int main() {
    int i,j,res;
    res = 0;
    for(i = 0; i < 4; i = i + 1)
    {
        for(j = 0; j < 30; j = j + 1)
        {
            res = res + 1;
            if (j == i * 2)
                break;
        }
    }
    print res;
}
