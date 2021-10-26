int main()
{
    int ptr1 = malloc(5),
        ptr2 = malloc(5),
        ptr3 = malloc(5),
		ptr4 = malloc(5);

    free(ptr1);
    int ptr5 = malloc(3);
    println(*0 < ptr5 && ptr5 < ptr2);
	
    free(ptr3);
    int ptr6 = malloc(10);
    println(ptr2 < ptr6 && ptr6 < ptr4);
}


