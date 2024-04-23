#include <boost/mpl/assert.hpp>
#include <vector>
#include <nt2/sdk/meta/is_iterator.hpp>

using nt2::meta::is_iterator;

int main()
{
  BOOST_MPL_ASSERT(( is_iterator<int*>        ));
  BOOST_MPL_ASSERT(( is_iterator< std::vector<char**>::iterator > ));
  BOOST_MPL_ASSERT(( is_iterator< std::vector<float>::reverse_iterator > ));
  BOOST_MPL_ASSERT_NOT(( is_iterator<int>            ));
  BOOST_MPL_ASSERT_NOT(( is_iterator<int(*)(double)> ));
}
