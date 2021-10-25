int main() {
    int a,b,c,d,e,f,g,h,i,res;
    a=b=c=d=e=f=g=h=i=res=0;
    while(a<4)
    {
        a=a+1;
        b=0;
        while(b<4)
        {
            b=b+1;
            c=0;
            while(c<4)
            {
                c=c+1;
                d=0;
                while(d<4)
                {
                    d=d+1;
                    e=0;
                    while(e<4)
                    {
                        e=e+1;
                        f=0;
                        while(f<4)
                        {
                            f=f+1;
                            g=0;
                            while(g<4)
                            {
                                g=g+1;
                                h=0;
                                while(h<4)
                                {
                                    h=h+1;
                                    res = res + 1;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    print res;
}
