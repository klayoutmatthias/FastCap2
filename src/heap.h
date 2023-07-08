
#if !defined(heap_H)
#define heap_H

#include <cstdlib>
#include <memory>

//  types of memory usage by multipole matrix type
enum MemoryType
{
  AQ2M = 0,
  AQ2L = 1,
  AQ2P = 2,
  AL2L = 3,
  AM2M = 4,
  AM2L = 5,
  AM2P = 6,
  AL2P = 7,
  AQ2PD = 8,
  AMSC = 9,
  NumTypes = 10
};

struct HeapPrivate;

/**
 *  @brief A class providing an allocation heap
 *
 *  The heap is a alloc-only structure that is cleaned as a whole
 *  upon destruction.
 *  The heap offers allocation functionality as well as
 *  memory tracking.
 */
class Heap
{
private:
  class DestructorBase
  {
  public:
    virtual ~DestructorBase() { }
    virtual void destroy() = 0;
  };

  template<class T>
  class destructor_impl : public DestructorBase
  {
  public:
    destructor_impl(T *t) : DestructorBase(), mp_t(t) { }

    virtual void destroy()
    {
      if (mp_t) {
        mp_t->~T();
      }
      mp_t = 0;
    }

  private:
    T *mp_t;
  };

public:
  Heap();
  ~Heap();

  template <class T>
  T *alloc(size_t n, MemoryType type = AMSC)
  {
    return (T *)malloc(n * sizeof(T), type);
  }

  template <class T>
  T *create(MemoryType type = AMSC)
  {
    void *t = malloc(sizeof(T), type);
    new (t) T();
    register_destructor(new destructor_impl<T>((T *)t));
    return (T *)t;
  }

  void *malloc(size_t n, MemoryType type = AMSC);

  char *strdup(const char *str, MemoryType type = AMSC);
  double **mat(int n, int m, MemoryType = AMSC);

  size_t memory(MemoryType type) const;
  size_t total_memory() const;

private:
  friend struct HeapPrivate;

  HeapPrivate *mp_data;
  size_t m_memory [NumTypes];

  void register_destructor(DestructorBase *);

  Heap(const Heap &);
  Heap &operator=(const Heap &);
};

#endif // HEAP_H
