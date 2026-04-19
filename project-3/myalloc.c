// Madison Neiswonger
// CS 474

#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Please do not change the order or type of the fields in struct block

struct block {
    int size;        // bytes
    int in_use;      // bool
    struct block *next;
};

// Use this for the pointer to the head of your list, AKA the value
// returned by `mmap()`:
static struct block *head = NULL;

// How much to allocate with `mmap()`
#define MEM_SIZE 1024

#define ALIGNMENT 16   // Must be power of 2
#define GET_PAD(x) ((ALIGNMENT - 1) - ((x - 1) & (ALIGNMENT - 1)))

// Get padded size for an integer
#define PADDED_SIZE(x) ((x) + GET_PAD(x))

// Get padded size of a type
#define PADDED_SIZEOF(x) (sizeof(x) + GET_PAD(sizeof (x)))

// Compute an offset pointer from a source pointer in bytes
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

/**
 * Allocate `size` bytes.
 *
 * Returns a pointer to the newly-allocated region, or `NULL` if there
 * is no more space.
 */
void *myalloc(int size)
{
    // One time initialization on first call
    if (head == NULL) {
        head = mmap(NULL, MEM_SIZE, PROT_READ|PROT_WRITE, MAP_ANON|MAP_PRIVATE, -1, 0);

        head->size = MEM_SIZE - PADDED_SIZEOF(struct block);
        head->in_use = 0;
        head->next = NULL;
    }

    int padded = PADDED_SIZE(size);
    int block_size = PADDED_SIZEOF(struct block);

    // Walk the list looking for a free block that's big enough
    struct block *b = head;

    while (b != NULL) {
        if (!b->in_use && b->size >= padded) {
            if (b->size >= padded + block_size + ALIGNMENT) {
                // Split the block
                struct block *new_block = PTR_OFFSET(b, block_size + padded);
                new_block->size = b->size - padded - block_size;
                new_block->in_use = 0;
                new_block->next = b->next;

                b->size = padded;
                b->in_use = 1;
                b->next = new_block;
            } else {
                b->in_use = 1;
            }

            return PTR_OFFSET(b, block_size);
        }

        b = b->next;
    }

    return NULL;
}

/**
 * Free a previously-allocated region.
 *
 * It is undefined behavior to pass any pointer to this function that
 * was not obtained via `myalloc()`.
 */
void myfree(void *p)
{
  int block_size = PADDED_SIZEOF(struct block);

  struct block *b = PTR_OFFSET(p, -block_size);
  b->in_use = 0;

  // Coalesce
  struct block *cur = head;
      while (cur != NULL) {
          if (!cur->in_use && cur->next != NULL && !cur->next->in_use) {
              // Merge cur and cur->next
              cur->size = cur->size + block_size + cur->next->size;
              cur->next = cur->next->next;
          } else {
              cur = cur->next;
        }
    }
}

// ---------------------------------------------------------
// No mods past this point, please
// ---------------------------------------------------------

/**
 * Print out the linked list.
 */
void print_data(void)
{
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        printf("[%d,%s]", b->size, b->in_use? "used": "free");
        if (b->next != NULL) {
            printf(" -> ");
        }
        fflush(stdout);

        b = b->next;
    }

    printf("\n");
}

/**
 * Parse a numeric command line argument.
 */
int parse_num_arg(char *progname, char *s)
{
    char *end;

    int value = strtol(s, &end, 10);

    if (*end != '\0') {
        fprintf(stderr, "%s: failed to parse numeric argument \"%s\"\n", progname, s);
        exit(1);
    }

    return value;
}

/*
 * Usage:
 *
 * You can specify the following commands:
 *
 * p       - print the list
 * a size  - allocate `size` bytes
 * f num   - free allocation number `num`
 *
 * For example, if we run this:
 *
 *   ./myalloc a 64 a 128 p f 2 p f 1 p
 *
 * Allocation #1 of 64 bytes occurs
 * Allocation #2 of 128 bytes occurs
 * The list is printed
 * Allocation #2 is freed (the 128-byte chunk)
 * The list is printed
 * Allocation #1 is freed (the 64-byte chunk)
 * The list is printed
 *
 * Failed allocations aren't counted for the 'f' argument, for example:
 *
 *   ./myalloc a 999999 f 1
 *
 * is an error, since that allocation will have failed.
 */
int main(int argc, char *argv[])
{
    if (argc == 1) {
        fprintf(stderr, "usage: %s [p|a size|f index] ...\n", argv[0]);
        return 1;
    }

    int i = 1;

    // This is how many allocs we can support on the command line
    void *ptr[128];
    int ptr_count = 0;

    while (i < argc) {
        if (strcmp(argv[i], "p") == 0)
            print_data();

        else if (strcmp(argv[i], "a") == 0) {
            i++;

            int size = parse_num_arg(argv[0], argv[i]);
            
            void *p = myalloc(size); 

            if (p == NULL)
                printf("failed to allocate %d byte%s\n", size, size == 1? "": "s");
            else
                ptr[ptr_count++] = p;

        } else if (strcmp(argv[i], "f") == 0) {
            i++;

            if (argv[i] == NULL) {
                fprintf(stderr, "%s: missing num argument for 'f' command\n", argv[0]);
                return 1;
            }

            int index = parse_num_arg(argv[0], argv[i]);

            if (index < 1 || index > ptr_count) {
                fprintf(stderr, "%s: 'f' command: argument %d out of range\n", argv[0], index);
                return 1;
            }

            myfree(ptr[index - 1]);

        } else {
            fprintf(stderr, "%s: unknown command: %s\n", argv[0], argv[i]);
            return 1;
        }

        i++;
    }
}
