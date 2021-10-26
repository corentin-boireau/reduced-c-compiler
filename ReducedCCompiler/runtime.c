int printn(int n)
{
    if (n < 0)
    {
        putchar(45);
        printn(-n);
    }
    else
    {
        if (n / 10 != 0)
            printn(n / 10);
        putchar(48 + n % 10);
    }
}

int println(int n)
{
    printn(n);
    putchar(10);
}

/*
* A block is composed of two size cells (s_cell), one at the
* beginning of the block, the other at the end.
* The data is contained between the 2 s_cells.
*
* The size stored in s_cells is the size of the data (excluding s_cells)
* Let s be the value stored in the s_cell:
*   - The block size is |s| (absolute value)
*   - if s > 0 then the block is free
*   - else the block is allocated
*
* A free block contains no data but instead stores pointers to :
*   - the next free block (N_CELL) stored in the first cell of the data space
*   - the previous free block (p_cell) stored in the last cell of the data space
* If there is no previous/next block, -1 is stored
*
* S_CELL_BEG and N_CELL are always at the same offset from the block start
* Since since s_cell_end and p_cell depends on the block size, they have
* to be recomputed for each block
*
* Pointers to block always contain the address of its first s_cell
*/

int _Init()
{
    int HEAP_SIZE = 16384; // 2^14
    int S_CELL_SIZE = 1;
    int INVALID_POINTER = -1;

    // Heap initialization
    int heap_start = *0;
    int first_free_block = heap_start + 1;
    *heap_start = first_free_block;

    // Initialization of the first free block
    int block_size = HEAP_SIZE - 2 * S_CELL_SIZE;
    int S_CELL_BEG = 0;
    int s_cell_end = S_CELL_SIZE + block_size;
    int N_CELL = S_CELL_SIZE;
    int p_cell = S_CELL_SIZE + block_size - 1;

    first_free_block[S_CELL_BEG] = block_size;
    first_free_block[s_cell_end] = block_size;
    first_free_block[N_CELL] = INVALID_POINTER;
    first_free_block[p_cell] = INVALID_POINTER;

    return 0;
}

int malloc(int size)
{
    int NULL = 0;
    int INVALID_POINTER = -1;
    int S_CELL_SIZE = 1;
    int S_CELL_BEG = 0;
    int N_CELL = S_CELL_SIZE;
    int BLOCK_MIN_SIZE = 2;

    if (size < 1)
        return NULL;

    // Every block has a minimum size of two to store prev and next pointers
    if (size < BLOCK_MIN_SIZE)
        size = BLOCK_MIN_SIZE;

    int ptr = NULL;

    int heap_start = *0;
    int free_block = *heap_start;
    while (free_block != INVALID_POINTER)
    {
        // The first cell of the block contains its size
        if (free_block[S_CELL_BEG] >= size)
            break;
        free_block = free_block[N_CELL];
    }
    if (free_block != INVALID_POINTER)
    {
        int free_block_size = free_block[S_CELL_BEG];
        // Pointers handling
        int p_cell = S_CELL_SIZE + free_block_size - 1;
        int next_block = free_block[N_CELL];
        int prev_block = free_block[p_cell];

        int allocated_block = free_block;
        int s_cell_end = S_CELL_SIZE + size;
        allocated_block[S_CELL_BEG] = -size;
        allocated_block[s_cell_end] = -size;

        // Splitting a block in two requires to create 2 new size cells
        if (free_block_size - size < BLOCK_MIN_SIZE + 2 * S_CELL_SIZE)
        { // Remaining space can't contain a new block
            if (prev_block == INVALID_POINTER)
            {
                *heap_start = next_block;
            }
            else
            {
                prev_block[N_CELL] = next_block;
            }

            if (next_block != INVALID_POINTER)
            {
                int next_block_size = next_block[S_CELL_BEG];
                int next_block_p_cell = S_CELL_BEG + next_block_size - 1;

                next_block[next_block_p_cell] = prev_block;
            }
        }
        else
        { // Remaining space is converted in a new free block
            int new_free_block = allocated_block + size + 2 * S_CELL_SIZE;
            int new_free_block_size = free_block_size - size - 2 * S_CELL_SIZE;
            int nfb_s_cell_end = S_CELL_SIZE + new_free_block_size;
            int nfb_p_cell = nfb_s_cell_end - 1;

            new_free_block[S_CELL_BEG] = new_free_block_size;
            new_free_block[nfb_s_cell_end] = new_free_block_size;

            new_free_block[N_CELL] = next_block;
            new_free_block[nfb_p_cell] = prev_block;

            if (prev_block == INVALID_POINTER)
            {
                *heap_start = new_free_block;
            }
            else
            {
                prev_block[N_CELL] = new_free_block;
            }

            if (next_block != INVALID_POINTER)
            {
                int next_block_size = next_block[S_CELL_BEG];
                int next_block_p_cell = S_CELL_BEG + next_block_size - 1;

                next_block[next_block_p_cell] = new_free_block;
            }
        }

        ptr = allocated_block + S_CELL_SIZE;
    }

    return ptr;
}

int free(int ptr)
{
    int NULL = 0;
    int INVALID_POINTER = -1;
    int S_CELL_SIZE = 1;
    int S_CELL_BEG = 0;
    int N_CELL = S_CELL_SIZE;
    int BLOCK_MIN_SIZE = 2;

    /* According to "man 3 free" : If ptr is NULL, no operation is performed. */
    if (ptr == NULL)
        return;

    /*
     * We suppose ptr has been returned by a call to malloc() and hasn't
     * be freed yet. Otherwise undefined behavior occurs
     */
    int block_to_free = ptr - S_CELL_SIZE;
    int block_size = -block_to_free[S_CELL_BEG]; // size is negative in an allocated block
    int s_cell_end = S_CELL_SIZE + block_size;
    int p_cell = s_cell_end - 1;

    block_to_free[S_CELL_BEG] = block_size;
    block_to_free[s_cell_end] = block_size;

    int heap_start = *0;
    int first_free_block = *heap_start;
    block_to_free[N_CELL] = first_free_block;
    block_to_free[p_cell] = INVALID_POINTER;

    if (first_free_block != INVALID_POINTER)
    {
        int ff_block_size = first_free_block[S_CELL_BEG];
        int ff_p_cell = S_CELL_SIZE + ff_block_size - 1;
        first_free_block[ff_p_cell] = block_to_free;
    }

    *heap_start = block_to_free;
}

int print_free_blocks_list()
{
    int INVALID_POINTER = -1;
    int S_CELL_SIZE = 1;
    int S_CELL_BEG = 0;
    int N_CELL = S_CELL_SIZE;

    int heap_start = *0;
    int free_block = *heap_start;
    putchar(72);
    while (free_block != INVALID_POINTER)
    {
        putchar(32);
        putchar(45);
        putchar(62);
        putchar(32);
        printn(free_block - heap_start);
        putchar(40);
        printn(free_block[S_CELL_BEG]);
        putchar(41);
        free_block = free_block[N_CELL];
    }
    putchar(10);
}

int memdump(int start, int end, int line_width)
{
    int i;
    putchar(38);
    putchar(9);
    for (i = 0;  i < line_width; i = i + 1)
    {
        printn(i);
        putchar(9);
    }
    putchar(10);
    for (i = 0;  i < line_width + 1; i = i + 1)
    {
        putchar(45);
        putchar(45);
        putchar(45);
        putchar(9);
    }
        
    int cell_addr;
    for (cell_addr = start; cell_addr < end; cell_addr = cell_addr + 1)
    {
        if ((cell_addr - start) % line_width == 0)
        {
            putchar(10);
            printn(cell_addr);
            putchar(58);
            putchar(9);
        }
        printn(*cell_addr);
        putchar(9);
    }
}

