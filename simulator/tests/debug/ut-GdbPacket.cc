
#include <boost/test/unit_test.hpp>

#include "debug/GdbPacket.h"

#include <boost/scoped_ptr.hpp>

using namespace patmos;

BOOST_AUTO_TEST_CASE( CreateEmptyPacketReturnsCorrectContent )
{
  GdbPacket empty;
  BOOST_REQUIRE_EQUAL(empty.GetContent(), "");
}

BOOST_AUTO_TEST_CASE( CreateEmptyPacketReturnsCorrectChecksum )
{
  GdbPacket empty;
  BOOST_REQUIRE_EQUAL(empty.GetChecksum(), 0);
}

BOOST_AUTO_TEST_CASE( CreatePacketWithEmptyContentReturnsCorrectContent )
{
  GdbPacket blank("",0);
  BOOST_REQUIRE_EQUAL(blank.GetContent(), "");
}

BOOST_AUTO_TEST_CASE( CreatePacketWithEmptyContentReturnsCorrectChecksum )
{
  GdbPacket blank("",0);
  BOOST_REQUIRE_EQUAL(blank.GetChecksum(), 0);
}

BOOST_AUTO_TEST_CASE( CreateNonEmptyPacketReturnsCorrectContent )
{
  const std::string packetContent = "non empty packet body";

  GdbPacket packet(packetContent, 0);
  BOOST_REQUIRE_EQUAL(packet.GetContent(), packetContent);
}

BOOST_AUTO_TEST_CASE( CreateNonEmptyPacketReturnsCorrectChecksum )
{
  const std::string packetContent = "non empty packet body";
  const int checksum = 25;
  
  GdbPacket packet(packetContent, checksum);
  BOOST_REQUIRE_EQUAL(packet.GetChecksum(), checksum);
}

BOOST_AUTO_TEST_CASE( GetPacketStringReturnsCorrectString )
{
  const std::string packetContent = "test string";
  const int checksum = 83;
  const std::string packetString = "$test string#53";
  
  GdbPacket packet(packetContent, checksum);
  BOOST_REQUIRE_EQUAL(packet.GetPacketString(), packetString);
}

BOOST_AUTO_TEST_CASE( TestStringWithChecksum83isInvalid )
{
  const std::string packetContent = "test string";
  const int checksum = 83;
  
  GdbPacket packet(packetContent, checksum);
  BOOST_REQUIRE(!packet.IsValid());
}

BOOST_AUTO_TEST_CASE( TestStringWithChecksum119isValid )
{
  const std::string packetContent = "test string";
  const int checksum = 119;
  
  GdbPacket packet(packetContent, checksum);
  BOOST_REQUIRE(packet.IsValid());
}

BOOST_AUTO_TEST_CASE( CreateGdbPacketProducesCorrectChecksum )
{
  const std::string packetContent = "test string";
  const int checksum = 119;
  
  GdbPacket packet = CreateGdbPacket(packetContent);;
  BOOST_REQUIRE_EQUAL(packet.GetChecksum(), checksum);
}

BOOST_AUTO_TEST_CASE( CreateGdbPacketProducesCorrectString )
{
  const std::string packetContent = "test string";
  const std::string packetString = "$test string#77";
  
  GdbPacket packet = CreateGdbPacket(packetContent);;
  BOOST_REQUIRE_EQUAL(packet.GetPacketString(), packetString);
}

BOOST_AUTO_TEST_CASE( ParseGdbPacketProducesCorrectContent )
{
  const std::string packetContent = "test string";
  const std::string packetString = "$test string#77";
  
  GdbPacket packet = ParseGdbPacket(packetString);
  BOOST_REQUIRE_EQUAL(packet.GetContent(), packetContent);
}

BOOST_AUTO_TEST_CASE( ParseGdbPacketProducesCorrectChecksum )
{
  const int checksum = 119;
  const std::string packetString = "$test string#77";
  
  GdbPacket packet = ParseGdbPacket(packetString);
  BOOST_REQUIRE_EQUAL(packet.GetChecksum(), checksum);
}

