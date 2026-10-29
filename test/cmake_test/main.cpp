#include <boost/random.hpp>

int main()
{
  boost::mt19937 rng;
  boost::uniform_int<> six(1,6);
  int x = six(rng);       
  return (x >= 1 && x <= 6) ? 0 : 1;
}
