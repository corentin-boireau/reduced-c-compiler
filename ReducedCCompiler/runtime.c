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
*   - the next free block (n_cell) stored in the first cell of the data space
*   - the previous free block (p_cell) stored in the last cell of the data space
* If there is no previous/next block, -1 is stored
*
* Pointers to block always contain the address of its first s_cell
*/

int _Init()
{
    int HEAP_SIZE = 16384; // 2^14
    int S_CELL_SIZE = 1;

    // Heap initialization
    int heap_start = *0;
    int first_free_block = heap_start + 1;
    *heap_start = first_free_block;

    // Initialization of the first free block
    int block_size = HEAP_SIZE - 2 * S_CELL_SIZE;
    int s_cell_beg = 0;
    int s_cell_end = S_CELL_SIZE + block_size;
    int n_cell = S_CELL_SIZE;
    int p_cell = S_CELL_SIZE + block_size - 1;

    first_free_block[s_cell_beg] = block_size;
    first_free_block[s_cell_end] = block_size;
    first_free_block[n_cell] = -1;
    first_free_block[p_cell] = -1;

    return 0;
}

int malloc(int size)
{
    int NULL = 0;
    int S_CELL_SIZE = 1;
    int BLOCK_MIN_SIZE = 2;

    if (size < 1)
        return NULL;

    // Every block has a minimum size of two to store prev and next pointers
    if (size < BLOCK_MIN_SIZE)
        size = BLOCK_MIN_SIZE;

    int ptr = NULL;

    int heap_start = *0;
    int free_block = *heap_start;
    int s_cell_beg = 0;
    int n_cell = S_CELL_SIZE;
    while (free_block != -1)
    {
        // The first cell of the block contains its size
        if (free_block[s_cell_beg] >= size)
            break;
        free_block = free_block[n_cell];
    }
    if (free_block != -1)
    {
        int free_block_size = free_block[s_cell_beg];
        // Pointers handling
        int p_cell = S_CELL_SIZE + free_block_size - 1;
        int next_block = free_block[n_cell];
        int prev_block = free_block[p_cell];

        int allocated_block = free_block;
        int s_cell_end = S_CELL_SIZE + size;
        allocated_block[s_cell_beg] = -size;
        allocated_block[s_cell_end] = -size;

        // Splitting a block in two requires to create 2 new size cells
        if (free_block_size - size < BLOCK_MIN_SIZE + 2 * S_CELL_SIZE)
        { // Remaining space can't contain a new block
            if (prev_block == -1)
            {
                *heap_start = next_block;
            }
            else
            {
                prev_block[n_cell] = next_block;
            }

            if (next_block != -1)
            {
                int next_block_size = next_block[s_cell_beg];
                int next_block_p_cell = s_cell_beg + next_block_size - 1;

                next_block[next_block_p_cell] = prev_block;
            }
        }
        else
        { // Remaining space is converted in a new free block
            int new_free_block = allocated_block + size + 2 * S_CELL_SIZE;
            int new_free_block_size = free_block_size - size;
            int nfb_s_cell_end = S_CELL_SIZE + new_free_block_size;

            new_free_block[s_cell_beg] = new_free_block_size;
            new_free_block[nfb_s_cell_end] = new_free_block_size;

            if (prev_block == -1)
            {
                *heap_start = new_free_block;
            }
            else
            {
                prev_block[n_cell] = next_block;
            }

            if (next_block != -1)
            {
                int next_block_size = next_block[s_cell_beg];
                int next_block_p_cell = s_cell_beg + next_block_size - 1;

                next_block[next_block_p_cell] = prev_block;
            }

        }

        ptr = free_block + 1;
    }

    return ptr;
}

int free(int ptr)
{
    return 0;
}

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
