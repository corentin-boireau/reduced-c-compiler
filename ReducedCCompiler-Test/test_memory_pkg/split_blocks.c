int main()
{
    int ptr1 = malloc(10);
    int ptr2 = malloc(10);
    int ptr3 = malloc(10);
    int ptr4 = malloc(10);

    print_free_blocks_list();

    free(ptr1);
    print_free_blocks_list();
    int ptr5 = malloc(7);
    print_free_blocks_list();

    free(ptr3);
    print_free_blocks_list();
    int ptr6 = malloc(6);
    print_free_blocks_list();
}
