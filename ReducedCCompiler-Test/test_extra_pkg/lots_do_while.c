int main() {
    int a,b,c,d,e,f,g,h,i,res;
    a=b=c=d=e=f=g=h=i=res=0;
    do
    {
        a=a+1;
        b=0;
        do
        {
            b=b+1;
            c=0;
            do
            {
                c=c+1;
                d=0;
                do
                {
                    d=d+1;
                    e=0;
                    do
                    {
                        e=e+1;
                        f=0;
                        do
                        {
                            f=f+1;
                            g=0;
                            do
                            {
                                g=g+1;
                                h=0;
                                do
                                {
                                    h=h+1;
                                    res = res + 1;
                                } while(h<4);
                            } while(g<4);
                        } while(f<4);
                    } while(e<4);
                } while(d<4);
            } while(c<4);
        } while(b<4);
    } while(a<4);
    print res;
}
