int main()
{
    int NULL = 0;

    println(malloc(-32) == NULL);

    println(malloc(0) == NULL);

    int ptr1 = malloc(1);
    // block min size is 2
    println(ptr1[-1] == -2);

    int ptr_inf = malloc(2122480999);
    println(ptr_inf == NULL);
}
