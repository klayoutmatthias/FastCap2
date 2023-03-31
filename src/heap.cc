#include "heap.h"

#include <vector>
#include <cstdlib>

struct HeapPrivate
{
  HeapPrivate () { }
  ~HeapPrivate ()
  {
    for (auto p = ptrs.begin(); p != ptrs.end(); ++p) {
      delete *p;
    }
  }

  std::vector<char *> ptrs;
};

Heap::Heap()
  : mp_data(0)
{
  for (unsigned int i = 0; i < NumTypes; ++i) {
    m_memory[i] = 0;
  }
}

Heap::~Heap()
{
  delete mp_data;
  mp_data = 0;
}

void *
Heap::malloc(size_t n, MemoryType type)
{
  if (! mp_data) {
    mp_data = new HeapPrivate ();
  }
  char *d = (char *)::malloc(n);
  mp_data->ptrs.push_back (d);
  if (type >= 0 && type < NumTypes) {
    m_memory[type] += type;
  }

  return d;
}

size_t
Heap::memory(MemoryType type) const
{
  if (type >= 0 && type < NumTypes) {
    return m_memory[type];
  } else {
    return 0;
  }
}

size_t
Heap::total_memory() const
{
  size_t n = 0;
  for (unsigned int i = 0; i < NumTypes; ++i) {
    n += m_memory[i];
  }
  return n;
}
