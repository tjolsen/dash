#ifndef DASH__ALGORITHM__GENERATE_H__
#define DASH__ALGORITHM__GENERATE_H__

#include <dash/iterator/GlobIter.h>
#include <dash/algorithm/LocalRange.h>
#include <dash/algorithm/Operation.h>

#include <dash/dart/if/dart_communication.h>

#include <algorithm>


namespace dash {

/**
 * Assigns each element in range [first, last) a value generated by the
 * given function object g.
 *
 * Being a collaborative operation, each unit will invoke the given
 * function on its local elements only.
 *
 * \tparam      ElementType    Type of the elements in the sequence
 *                             invoke, deduced from parameter \c gen
 * \tparam      UnaryFunction  Unary function with signature
 *                             \c ElementType(void)
 *
 * \complexity  O(d) + O(nl), with \c d dimensions in the global iterators'
 *              pattern and \c nl local elements within the global range
 *
 * \ingroup     DashAlgorithms
 */
template <
    typename ElementType,
    class    PatternType,
    class    UnaryFunction >
void generate (
  /// Iterator to the initial position in the sequence
  GlobIter<ElementType, PatternType> first,
  /// Iterator to the final position in the sequence
  GlobIter<ElementType, PatternType> last,
  /// Generator function
  UnaryFunction                      gen) {
  /// Global iterators to local range:
  auto lrange = dash::local_range(first, last);
  auto lfirst = lrange.begin;
  auto llast  = lrange.end;

  std::generate(lfirst, llast, gen);
}

/**
 * Assigns each element in range [first, last) a value generated by the
 * given function object g. The index passed to the function is
 * a global index.
 *
 * Being a collaborative operation, each unit will invoke the given
 * function on its local elements only.
 *
 * \tparam      ElementType    Type of the elements in the sequence
 *                             invoke, deduced from parameter \c gen
 * \tparam      UnaryFunction  Unary function with signature
 *                             \c ElementType(index_t)
 *
 * \complexity  O(d) + O(nl), with \c d dimensions in the global iterators'
 *              pattern and \c nl local elements within the global range
 *
 * \ingroup     DashAlgorithms
 */
template <
    typename ElementType,
    class    PatternType,
    class    UnaryFunction >
void generate_with_index(
  /// Iterator to the initial position in the sequence
  GlobIter<ElementType, PatternType> first,
  /// Iterator to the final position in the sequence
  GlobIter<ElementType, PatternType> last,
  /// Generator function
  UnaryFunction                      gen) {
  /// Global iterators to local index range:
  auto index_range  = dash::local_index_range(first, last);
  auto lbegin_index = index_range.begin;
  auto lend_index   = index_range.end;

  if (lbegin_index != lend_index) {
    // Pattern from global begin iterator:
    auto & pattern    = first.pattern();
    auto first_offset = first.pos();
    // Iterate local index range:
    for (auto lindex = lbegin_index;
         lindex != lend_index;
         ++lindex) {
      auto gindex     = pattern.global(lindex);
      auto element_it = first + (gindex - first_offset);
      *element_it = gen(gindex);
    }
  }
}

} // namespace dash

#endif // DASH__ALGORITHM__GENERATE_H__
