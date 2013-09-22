
#include <boost/test/unit_test.hpp>

#include "debug/GdbException.h"

BOOST_AUTO_TEST_CASE( CreateGdbExceptionProducesCorrectWhatMessage )
{
  const patmos::GdbException ex("test");
  const std::string expectedWhatMessage = "Error: GdbServer: test";
  BOOST_REQUIRE_EQUAL(ex.what(), expectedWhatMessage);
}

