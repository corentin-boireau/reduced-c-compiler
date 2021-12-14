int main()
{
    int ptr_array = malloc(10);
    for (int i = 0; i < 10; i = i + 1)
        ptr_array[i] = malloc(5);
    for (int i = 0; i < 9; i = i + 1)
    {
        free(ptr_array[i]);
        print_free_blocks_list();
    }
}
