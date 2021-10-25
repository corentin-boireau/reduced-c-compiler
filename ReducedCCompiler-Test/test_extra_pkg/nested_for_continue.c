int main() {
    int i,j,res;
    i=res=0;
    for(i = 0; i < 4; i = i + 1)
        for(j = 0; j < 30; j = j + 1)
        {
            if (j > 12)
                continue;
            res = res + 1;
        }

    print res;
}
