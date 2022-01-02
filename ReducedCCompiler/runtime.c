int printn(int n)
{
    if (n < 0)
    {
        putchar('-');
        int last_digit = -(n % 10);
        printn(-(n / 10));
        putchar('0' + last_digit);
    }
    else
    {
        if (n / 10 != 0)
            printn(n / 10);
        putchar('0' + n % 10);
    }
}

int println(int n)
{
    printn(n);
    putchar('\n');
}

/*
 * Same as 'scanf("%d")' but returns the number of digits read instead of the
 * number of input items successfully matched and assigned.
 */
int scann(int ptr_n)
{
    int nb_digits = 0;

    int sign = 1;
    int value = 0;
    int character = getchar();
    if (('0' <= character && character <= '9') || character == '-')
    {
        if (character == '-')
        {
            sign = -1;
            character = getchar();
        }

        while (character != '\n'
               && '0' <= character && character <= '9')
        {
            nb_digits = nb_digits + 1;
            value = value * 10 + character - '0';
            character = getchar();
        }
        if (character > 0 && nb_digits > 0)
        {
            *ptr_n = value * sign;
        }
    }

    return nb_digits;
}

/*
* A block is composed of two size cells (s_cell), one at the
* beginning of the block, the other at the end.
* The data is contained between the 2 s_cells.
*
* The size stored in s_cells is the total size of the block (including s_cells)
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
* S_CELL_BEG and N_CELL are always at the same offset from the start of the block
* Since since s_cell_end and p_cell depends on the block size, they have
* to be recomputed for each block
*
* Pointers to block always contain the address of its first s_cell
*/

const int HEAP_SIZE = 16384; // 2^14
int HEAP_START;
const int INVALID_POINTER = -1;
const int NULL = 0;
const int S_CELL_SIZE = 1;
const int S_CELL_BEG = 0;
const int N_CELL = 1;
const int BLOCK_SIZE_MIN = 4;
const int DATA_SIZE_MIN = 2;

const int INT_MAX = 2147483647;
const int INT_MIN = -2147483647 - 1;

int block_to_data_size(int block_size)
{
    return block_size - 2 * S_CELL_SIZE;
}

int data_to_block_size(int data_size)
{
    return data_size + 2 * S_CELL_SIZE;
}

int compute_s_cell_end(int block_size)
{
    return block_size - S_CELL_SIZE;
}

int compute_p_cell(int block_size)
{
    return block_size - S_CELL_SIZE - 1;
}

int data_to_block_pointer(int data_pointer)
{
    return data_pointer - S_CELL_SIZE;
}

int block_to_data_pointer(int block_pointer)
{
    return block_pointer + S_CELL_SIZE;
}

int _Init()
{
    HEAP_START = *0;

    // Heap initialization
    int first_free_block = HEAP_START + 1;
    *HEAP_START = first_free_block;

    // Initialization of the first free block
    int block_size = HEAP_SIZE;
    int s_cell_end = compute_s_cell_end(block_size);
    int p_cell = compute_p_cell(block_size);

    first_free_block[S_CELL_BEG] = block_size;
    first_free_block[s_cell_end] = block_size;
    first_free_block[N_CELL] = INVALID_POINTER;
    first_free_block[p_cell] = INVALID_POINTER;
}

int malloc(int size)
{
    if (size < 1)
        return NULL;

    // Every block has a minimum data size to store prev and next pointers
    if (size < DATA_SIZE_MIN)
        size = DATA_SIZE_MIN;

    int ptr = NULL;
    int allocated_block_size = data_to_block_size(size);

    int free_block = *HEAP_START;
    while (free_block != INVALID_POINTER && free_block[S_CELL_BEG] < allocated_block_size)
    {
        free_block = free_block[N_CELL];
    }
    if (free_block != INVALID_POINTER)
    {
        int free_block_size = free_block[S_CELL_BEG];
        // Pointers handling
        int p_cell = compute_p_cell(free_block_size);
        int next_block = free_block[N_CELL];
        int prev_block = free_block[p_cell];

        int allocated_block = free_block;
        int s_cell_end = compute_s_cell_end(allocated_block_size);
        allocated_block[S_CELL_BEG] = -allocated_block_size;
        allocated_block[s_cell_end] = -allocated_block_size;

        // Splitting a block in two requires to create 2 new size cells
        int remaining_size = free_block_size - allocated_block_size;
        if (remaining_size < BLOCK_SIZE_MIN)
        { // Remaining space can't contain a new block
            if (prev_block == INVALID_POINTER)
            {
                *HEAP_START = next_block;
            }
            else
            {
                prev_block[N_CELL] = next_block;
            }

            if (next_block != INVALID_POINTER)
            {
                int next_block_size = next_block[S_CELL_BEG];
                int next_block_p_cell = compute_p_cell(next_block_size);

                next_block[next_block_p_cell] = prev_block;
            }

            if (remaining_size > 0)
            {
                int dead_block = allocated_block + allocated_block_size;
                int db_s_cell_end = compute_s_cell_end(remaining_size);
                dead_block[S_CELL_BEG] = remaining_size;
                dead_block[db_s_cell_end] = remaining_size;
            }
        }
        else
        { // Remaining space is converted in a new free block
            int new_free_block = allocated_block + allocated_block_size;
            int new_free_block_size = remaining_size;
            int nfb_s_cell_end = compute_s_cell_end(new_free_block_size);
            int nfb_p_cell = compute_p_cell(new_free_block_size);

            new_free_block[S_CELL_BEG] = new_free_block_size;
            new_free_block[nfb_s_cell_end] = new_free_block_size;

            new_free_block[N_CELL] = next_block;
            new_free_block[nfb_p_cell] = prev_block;

            if (prev_block == INVALID_POINTER)
            {
                *HEAP_START = new_free_block;
            }
            else
            {
                prev_block[N_CELL] = new_free_block;
            }

            if (next_block != INVALID_POINTER)
            {
                int next_block_size = next_block[S_CELL_BEG];
                int next_block_p_cell = compute_p_cell(next_block_size);

                next_block[next_block_p_cell] = new_free_block;
            }
        }

        ptr = block_to_data_pointer(allocated_block);
    }

    return ptr;
}

int free(int ptr)
{
    /* According to "man 3 free" : If ptr is NULL, no operation is performed. */
    if (ptr == NULL)
        return;

    /*
     * We suppose ptr has been returned by a call to malloc() and hasn't
     * be freed yet. Otherwise undefined behavior occurs
     */
    int block_to_free = data_to_block_pointer(ptr);
    int block_size = -block_to_free[S_CELL_BEG]; // size is negative in an allocated block

    int new_free_block;
    int nfb_size;
    int nfb_next = *HEAP_START;
    int nfb_prev = INVALID_POINTER;
    int nfb_prev_n_cell = HEAP_START;

    int lneighbor_size = 0;
    if (block_to_free - S_CELL_SIZE != HEAP_START)
    { // Has a left neighbor
        lneighbor_size = block_to_free[S_CELL_BEG - S_CELL_SIZE];
        if (lneighbor_size < 0)
            lneighbor_size = 0;
        else if (lneighbor_size > BLOCK_SIZE_MIN)
        { // Left neighbor is free
            int lneighbor_block = block_to_free - lneighbor_size;
            nfb_next = lneighbor_block[N_CELL];
            nfb_prev = lneighbor_block[compute_p_cell(lneighbor_size)];
        }
    }

    int rneighbor_size = 0;
    if (block_to_free + block_size < HEAP_START + HEAP_SIZE)
    { // Has a right neighbor
        int rneighbor_block = block_to_free + block_size;
        rneighbor_size = rneighbor_block[S_CELL_BEG];
        if (rneighbor_size < 0)
            rneighbor_size = 0;
        else if (rneighbor_size > BLOCK_SIZE_MIN)
        { // Right neighbor is free
            //Remove rneighbor from the linked list
            int rn_next = rneighbor_block[N_CELL];
            int rn_prev = rneighbor_block[compute_p_cell(rneighbor_size)];
            if (rn_next != INVALID_POINTER)
            {
                int rn_next_size = rn_next[S_CELL_BEG];
                rn_next[compute_p_cell(rn_next_size)] = rn_prev;
            }

            if (rn_prev != INVALID_POINTER)
            {
                rn_prev[N_CELL] = rn_next;

                if (lneighbor_size > 0 // we are merging the two neighbors
                    && nfb_next == rneighbor_block) // lneighbor's next block is rneighbor
                {
                    nfb_next = rn_next;
                }
            }
            else // rneighbor is the first free block
            {
                if (lneighbor_size > 0)
                { // we are merging the two neighbors
                    *HEAP_START = rn_next;
                }
                else // we are only merging the rneighbor
                {
                    nfb_next = rn_next;
                }
            }
        }
    }

    new_free_block = block_to_free - lneighbor_size;
    nfb_size = block_size + lneighbor_size + rneighbor_size;

    int nfb_s_cell_end = compute_s_cell_end(nfb_size);
    int nfb_p_cell = compute_p_cell(nfb_size);

    new_free_block[S_CELL_BEG] = nfb_size;
    new_free_block[nfb_s_cell_end] = nfb_size;

    new_free_block[N_CELL] = nfb_next;
    new_free_block[nfb_p_cell] = nfb_prev;

    if (nfb_next != INVALID_POINTER)
    {
        int nfb_next_size = nfb_next[S_CELL_BEG];
        int nfb_next_p_cell = compute_p_cell(nfb_next_size);
        nfb_next[nfb_next_p_cell] = new_free_block;
    }

    *nfb_prev_n_cell = new_free_block;
}

int print_free_blocks_list()
{
    int free_block = *HEAP_START;
    putchar('H');
    while (free_block != INVALID_POINTER)
    {
        putchar(' ');
        putchar('-');
        putchar('>');
        putchar(' ');
        printn(free_block - HEAP_START);
        putchar('(');
        printn(free_block[S_CELL_BEG]);
        putchar(')');
        free_block = free_block[N_CELL];
    }
    putchar('\n');
}

int memdump(int start, int end, int line_width)
{
    int i;
    putchar('&');
    putchar('\t');
    for (i = 0;  i < line_width; i += 1)
    {
        printn(i);
        putchar('\t');
    }
    putchar('\n');
    for (i = 0;  i < line_width + 1; i += 1)
    {
        putchar('-');
        putchar('-');
        putchar('-');
        putchar('\t');
    }

    int cell_addr;
    for (cell_addr = start; cell_addr < end; cell_addr += 1)
    {
        if ((cell_addr - start) % line_width == 0)
        {
            putchar('\n');
            printn(cell_addr);
            putchar(':');
            putchar('\t');
        }
        printn(*cell_addr);
        putchar('\t');
    }
}
