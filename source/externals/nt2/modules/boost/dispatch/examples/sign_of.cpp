#include <boost/mpl/assert.hpp>
#include <boost/dispatch/meta/sign_of.hpp>
#include <boost/type_traits/is_same.hpp>

using boost::is_same;
using boost::dispatch::meta::sign_of;

int main()
{
  BOOST_MPL_ASSERT(( is_same<unsigned, sign_of<unsigned int>::type> ));
  BOOST_MPL_ASSERT(( is_same<signed,   sign_of<int>::type>          ));
  BOOST_MPL_ASSERT(( is_same<unsigned, sign_of<bool>::type>         ));
  BOOST_MPL_ASSERT(( is_same<signed,   sign_of<float>::type>        ));
}
