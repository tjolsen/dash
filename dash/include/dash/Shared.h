#ifndef DASH__SHARED_H_
#define DASH__SHARED_H_

#include <dash/dart/if/dart_types.h>

#include <dash/GlobMem.h>
#include <dash/GlobIter.h>
#include <dash/GlobRef.h>
#include <dash/Allocator.h>
#include <dash/Atomic.h>

#include <memory>


namespace dash {

/**
 * Shared access to a value in global memory across a team.
 *
 * \tparam  ElementType  The type of the shared value.
 */
template<typename ElementType>
class Shared
{
private:
  typedef Shared<ElementType>
    self_t;

public:
  typedef ElementType                                value_type;
  typedef size_t                                      size_type;
  typedef size_t                                difference_type;

  typedef       GlobRef<value_type>                   reference;
  typedef const GlobRef<value_type>             const_reference;

  typedef       GlobPtr<value_type>                     pointer;
  typedef const GlobPtr<value_type>               const_pointer;

  typedef dash::Atomic<ElementType>
    atomic_type;

private:
  typedef dash::GlobMem<
            value_type,
            dash::allocator::LocalAllocator<value_type> >
    GlobMem_t;

  template<typename T_>
  friend void swap(Shared<T_> & a, Shared<T_> & b);

public:
  atomic_type atomic;

public:
  /**
   * Constructor, allocates shared value at single unit in specified team.
   */
  Shared(
    /// Unit id of the shared value's owner.
    dart_unit_t   owner = 0,
    /// Team containing all units accessing the element in shared memory
    Team        & team  = dash::Team::All())
  : _team(&team),
    _owner(owner),
    _ptr(DART_GPTR_NULL)
  {
    DASH_LOG_DEBUG_VAR("Shared.Shared(team,owner)()", owner);
    // Shared value is only allocated at unit 0:
    if (_team->myid() == _owner) {
      DASH_LOG_DEBUG("Shared.Shared(team,owner)",
                     "allocating shared value in local memory");
      _globmem = std::make_shared<GlobMem_t>(1, team);
      _ptr     = _globmem->begin();
    }
    // Broadcast global pointer of shared value at unit 0 to all units:
    dart_bcast(
      &_ptr,
      sizeof(pointer),
	    _owner,
      _team->dart_id());
    atomic = atomic_type(_ptr.dart_gptr(), team);
    DASH_LOG_DEBUG_VAR("Shared.Shared(team,owner) >", _ptr);
  }

  /**
   * Destructor, frees shared memory.
   */
  ~Shared()
  {
    DASH_LOG_DEBUG("Shared.~Shared()");
    DASH_LOG_DEBUG("Shared.~Shared >");
  }

#if 0
  // For logging of assignment / move operations only.

  /**
   * Copy-constructor.
   */
  Shared(const self_t & other)
  : _team(other._team),
    _owner(other._owner),
    _globmem(other._globmem),
    _ptr(other._ptr)
  {
    DASH_LOG_DEBUG("Shared.Shared(other) >");
  }

  /**
   * Move-constructor. Transfers ownership from other instance.
   */
  Shared(self_t && other)
  : _team(other._team),
    _owner(other._owner),
    _ptr(other._ptr)
  {
    DASH_LOG_DEBUG("Shared.Shared(&& other)()");
    // move ownership of assigned instance's _globmem:
    _globmem       = other._globmem;
    // leave moved instance in valid state:
    other._globmem.reset();
    other._globmem = nullptr;
    DASH_LOG_DEBUG("Shared.Shared(&& other) >");
  }

  /**
   * Assignment operator.
   */
  self_t & operator=(const self_t & other)
  {
    DASH_LOG_DEBUG("Shared.=(other)()");
    _team    = other._team;
    _owner   = other._owner;
    _globmem = other._globmem;
    _ptr     = other._ptr;
    DASH_LOG_DEBUG("Shared.=(other) >");
    return *this;
  }

  /**
   * Move-assignment operator.
   */
  self_t & operator=(self_t && other)
  {
    DASH_LOG_DEBUG("Shared.=(&& other)()");
    _team          = other._team;
    _owner         = other._owner;
    _ptr           = other._ptr;
    // move ownership of assigned instance's _globmem:
    _globmem       = other._globmem;
    // leave moved instance in valid state:
    other._globmem = nullptr;
    other._globmem.reset();
    DASH_LOG_DEBUG("Shared.=(&& other) >");
  }
#else
  /**
   * Copy-constructor.
   */
  Shared(const self_t & other) = default;

  /**
   * Move-constructor. Transfers ownership from other instance.
   */
  Shared(self_t && other) = default;

  /**
   * Assignment operator.
   */
  self_t & operator=(const self_t & other) = default;

  /**
   * Move-assignment operator.
   */
  self_t & operator=(self_t && other) = default;
#endif

  /**
   * Set the value of the shared element.
   */
  void set(ElementType val)
  {
    DASH_LOG_DEBUG_VAR("Shared.set()", val);
    DASH_LOG_DEBUG_VAR("Shared.set",   _owner);
    DASH_LOG_DEBUG_VAR("Shared.set",   _ptr);
    DASH_ASSERT(
      !DART_GPTR_EQUAL(
        _ptr.dart_gptr(), DART_GPTR_NULL));
    *_ptr = val;
    DASH_LOG_DEBUG("Shared.set >");
  }

  /**
   * Get a reference on the shared value.
   */
  reference get()
  {
    DASH_LOG_DEBUG("Shared.cget()");
    DASH_LOG_DEBUG_VAR("Shared.cget", _owner);
    DASH_LOG_DEBUG_VAR("Shared.cget", _ptr);
    DASH_ASSERT(
      !DART_GPTR_EQUAL(
        _ptr.dart_gptr(), DART_GPTR_NULL));
    const_reference ref = *_ptr;
    DASH_LOG_DEBUG_VAR("Shared.cget >", static_cast<ElementType>(ref));
    return ref;
  }

  /**
   * Get a const reference on the shared value.
   */
  const_reference get() const
  {
    DASH_LOG_DEBUG("Shared.get()");
    DASH_LOG_DEBUG_VAR("Shared.get", _owner);
    DASH_LOG_DEBUG_VAR("Shared.get", _ptr);
    DASH_ASSERT(
      !DART_GPTR_EQUAL(
        _ptr.dart_gptr(), DART_GPTR_NULL));
    reference ref = *_ptr;
    DASH_LOG_DEBUG_VAR("Shared.get >", static_cast<ElementType>(ref));
    return ref;
  }

  /**
   * Synchronize units associated with the shared value.
   */
  void barrier()
  {
    DASH_ASSERT(_team != nullptr);
    _team->barrier();
  }

  /**
   * Get underlying DART global pointer of the shared variable.
   */
  inline dart_gptr_t dart_gptr() const noexcept
  {
    return _ptr.dart_gptr();
  }

private:
  dash::Team                   * _team    = nullptr;
  dart_unit_t                    _owner   = DART_UNDEFINED_UNIT_ID;
  std::shared_ptr<GlobMem_t>     _globmem = nullptr;
  pointer                        _ptr;

};

template<typename T>
void swap(
  dash::Shared<T> & a,
  dash::Shared<T> & b)
{
  using std::swap;
  swap(a._team,    b._team);
  swap(a._owner,   b._owner);
  swap(a._globmem, b._globmem);
  swap(a._ptr,     b._ptr);
}

} // namespace dash

#endif // DASH__SHARED_H_
