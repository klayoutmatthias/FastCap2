#include "heap.h"

#include <vector>
#include <cstdlib>
#include <cstring>

struct HeapPrivate
{
  HeapPrivate () { }
  ~HeapPrivate ()
  {
    for (auto d = destructors.begin(); d != destructors.end(); ++d) {
      (*d)->destroy();
      delete *d;
    }
    destructors.clear();
    for (auto p = ptrs.begin(); p != ptrs.end(); ++p) {
      ::free(*p);
    }
    ptrs.clear();
  }

  std::vector<char *> ptrs;
  std::vector<Heap::DestructorBase *> destructors;
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
    mp_data = new HeapPrivate();
  }
  char *d = (char *)::malloc(n);
  mp_data->ptrs.push_back (d);
  if (type >= 0 && type < NumTypes) {
    m_memory[type] += n;
  }

  bzero(d, n);

  return d;
}

void Heap::register_destructor(DestructorBase *destructor)
{
  if (! mp_data) {
    mp_data = new HeapPrivate();
  }
  mp_data->destructors.push_back(destructor);
}

char *Heap::strdup(const char *str, MemoryType type)
{
  if (! str) {
    return 0;
  } else {
    char *d = this->alloc<char>(strlen(str) + 1, type);
    strcpy(d, str);
    return d;
  }
}

double **Heap::mat(int n, int m, MemoryType type)
{
  double **d = this->alloc<double *>(n, type);
  for (int i = 0; i < n; ++i) {
    d[i] = this->alloc<double>(m, type);
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
