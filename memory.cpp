#include <iostream>
#include <cstdint>
struct Header
{
  int size;
  bool allocated;
  Header *prev;
  Header *next;
};

// Define amount of memory to allocate and the aignment size, which is 8.
constexpr int HEAP_SIZE = 1024 * 1024;
constexpr int ALIGN = 8;
alignas(ALIGN) static char heap[HEAP_SIZE];
Header *free_list_head = nullptr;

void create_heap()
{
  Header *initial = reinterpret_cast<Header *>(heap);
  initial->size = HEAP_SIZE - sizeof(Header);
  initial->allocated = false;
  initial->prev = nullptr;
  initial->next = nullptr;
  free_list_head = initial;
}

Header *find_free_block(int requested_size)
{
  Header *current = free_list_head;

  while (current != nullptr)
  {
    if (!current->allocated && current->size >= requested_size)
    {
      return current;
    }
    current = current->next;
  }
  return nullptr;
}
size_t align_up(size_t n, size_t align = ALIGN)
{
  size_t rem = n % align;
  return rem == 0 ? n : n + (align - rem);
}
void *allocate(int size)
{

  Header *block = find_free_block(size);
  if (block == nullptr)
  {
    return nullptr;
  }
  // Round user size up to alignment
  size = align_up(size);

  int total_size = block->size;
  int needed = size + (int)sizeof(Header);

  if (total_size > needed)
  {
    Header *new_block = (Header *)((char *)block + sizeof(Header) + size);
    int remaining_size = total_size - size - (int)sizeof(Header);

    new_block->size = remaining_size;
    new_block->allocated = false;

    new_block->prev = block->prev;
    new_block->next = block->next;

    if (new_block->prev != nullptr)
    {
      new_block->prev->next = new_block;
    }
    else
    {
      free_list_head = new_block;
    }

    if (new_block->next != nullptr)
    {
      new_block->next->prev = new_block;
    }
    block->size = size;
  }
  else
  {

    if (block->prev != nullptr)
    {
      block->prev->next = block->next;
    }
    else
    {
      free_list_head = block->next;
    }

    if (block->next != nullptr)
    {
      block->next->prev = block->prev;
    }
  }
  block->allocated = true;
  return (void *)((char *)block + sizeof(Header));
}

void free(void *ptr)
{
  if (ptr == nullptr)
    return;

  Header *block = (Header *)((char *)ptr - sizeof(Header));

  block->allocated = false;
  block->prev = nullptr;
  block->next = free_list_head;

  if (free_list_head != nullptr)
  {
    free_list_head->prev = block;
  }
  free_list_head = block;

  Header *next_phys = (Header *)((char *)block + sizeof(Header) + block->size);

  if ((char *)next_phys < heap + HEAP_SIZE && !next_phys->allocated)
  {
    // remove next_phys from free list
    if (next_phys->prev)
      next_phys->prev->next = next_phys->next;
    else
      free_list_head = next_phys->next;

    if (next_phys->next)
      next_phys->next->prev = next_phys->prev;

    // merge
    block->size += sizeof(Header) + next_phys->size;
  }
  Header *cur = (Header *)heap;
  Header *prev_phys = nullptr;

  while ((char *)cur < (char *)block)
  {
    prev_phys = cur;
    cur = (Header *)(((char *)cur + sizeof(Header)) + cur->size);
  }
  if (prev_phys && !prev_phys->allocated)
  {
    // remove block from free list
    if (block->prev)
      block->prev->next = block->next;
    else
      free_list_head = block->next;

    if (block->next)
      block->next->prev = block->prev;

    // merge
    prev_phys->size += sizeof(Header) + block->size;

    block = prev_phys;
  }
}

void print_free_list()
{
  std::cout << "Free list: \n";
  Header *cur = free_list_head;
  while (cur)
  {
    std::cout << "Block at " << (void *)cur
              << " size=" << cur->size
              << " allocated=" << cur->allocated << "\n";
    cur = cur->next;
    std::cout << "----\n";
  }
}

int main()
{
  create_heap();
  std::cout << "Initial state:\n";
  print_free_list();

  // Allocate 100 bytes
  void *a = allocate(100);
  std::cout << "After allocating 100:\n";
  print_free_list();

  // Allocate 200 bytes
  void *b = allocate(200);
  std::cout << "After allocating 200:\n";
  print_free_list();

  // Free the first block
  free(a);
  std::cout << "After freeing first block:\n";
  print_free_list();

  // Free the second block (should coalesce)
  free(b);
  std::cout << "After freeing second block (coalesced):\n";
  print_free_list();

  return 0;
}
