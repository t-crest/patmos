
#include <boost/test/unit_test.hpp>

#include "debug/GdbPacketHandler.h"
#include "debug/GdbConnection.h"

#include "TestConnection.h"

#include <boost/scoped_ptr.hpp>

using namespace patmos;

BOOST_AUTO_TEST_SUITE ( GdbPacketHandlerTests )

BOOST_AUTO_TEST_CASE( ReadFirstGdbPacketReturnsCorrectContent )
{
  const std::string packetContent = "$test string#77  $a bad checksum#00";
  const std::string firstPacket   = "test string";
  boost::scoped_ptr<GdbConnection> con(new test::TestConnection(packetContent));
  boost::scoped_ptr<GdbPacketHandler> handler(new GdbPacketHandler(*con));

  GdbPacket packet = handler->ReadGdbPacket();
  BOOST_REQUIRE_EQUAL(packet.GetContent(), firstPacket);
}

BOOST_AUTO_TEST_CASE( ReadFirstGdbPacketReturnsCorrectChecksum )
{
  const std::string packetContent = "$test string#77  $a bad checksum#00";
  const int checksum = 119;
  boost::scoped_ptr<GdbConnection> con(new test::TestConnection(packetContent));
  boost::scoped_ptr<GdbPacketHandler> handler(new GdbPacketHandler(*con));
  
  GdbPacket packet = handler->ReadGdbPacket();
  BOOST_REQUIRE_EQUAL(packet.GetChecksum(), checksum);
}

BOOST_AUTO_TEST_CASE( ReadFirstGdbPacketReturnsValidPacket )
{
  const std::string packetContent = "$test string#77  $a bad checksum#00";
  boost::scoped_ptr<GdbConnection> con(new test::TestConnection(packetContent));
  boost::scoped_ptr<GdbPacketHandler> handler(new GdbPacketHandler(*con));
  
  GdbPacket packet = handler->ReadGdbPacket();
  BOOST_REQUIRE(packet.IsValid());
}

BOOST_AUTO_TEST_CASE( ReadSecondGdbPacketReturnsCorrectContent )
{
  const std::string packetContent = "$test string#77  $a good checksum#9d";
  const std::string secondPacket   = "a good checksum";
  boost::scoped_ptr<GdbConnection> con(new test::TestConnection(packetContent));
  boost::scoped_ptr<GdbPacketHandler> handler(new GdbPacketHandler(*con));
  
  GdbPacket packet = handler->ReadGdbPacket();
  packet = handler->ReadGdbPacket();
  BOOST_REQUIRE_EQUAL(packet.GetContent(), secondPacket);
}

BOOST_AUTO_TEST_CASE( ReadSecondGdbPacketReturnsCorrectChecksum )
{
  const std::string packetContent = "$test string#77  $a good checksum#9d";
  const int checksum = 157;
  boost::scoped_ptr<GdbConnection> con(new test::TestConnection(packetContent));
  boost::scoped_ptr<GdbPacketHandler> handler(new GdbPacketHandler(*con));
  
  GdbPacket packet = handler->ReadGdbPacket();
  packet = handler->ReadGdbPacket();
  BOOST_REQUIRE_EQUAL(packet.GetChecksum(), checksum);
}

BOOST_AUTO_TEST_CASE( ReadSecondGdbPacketNeedsRetryAndReturnsCorrectContent )
{
  const std::string packetContent = "$test string#77  $a bad checksum#00  $a good checksum#9d";
  const std::string content = "a good checksum";
  boost::scoped_ptr<GdbConnection> con(new test::TestConnection(packetContent));
  boost::scoped_ptr<GdbPacketHandler> handler(new GdbPacketHandler(*con));
  
  GdbPacket packet = handler->ReadGdbPacket();
  packet = handler->ReadGdbPacket();
  BOOST_REQUIRE_EQUAL(packet.GetContent(), content);
}

BOOST_AUTO_TEST_CASE( WriteGdbPacketWithResponseProducesCorrectOutput )
{
  const std::string packetString = "$test string#77";
  const std::string packetContent = "test string";
  const std::string response = "+";
  boost::scoped_ptr<test::TestConnection> con(new test::TestConnection(response));
  boost::scoped_ptr<GdbPacketHandler> handler(new GdbPacketHandler(*con));

  GdbPacket packet = CreateGdbPacket(packetContent);
  handler->WriteGdbPacket(packet);

  BOOST_REQUIRE_EQUAL(con->GetOutputWritten(), packetString);
}

BOOST_AUTO_TEST_CASE( WriteGdbPacketWithRetryProducesCorrectOutput )
{
  const std::string packetString = "$test string#77$test string#77";
  const std::string packetContent = "test string";
  const std::string response = "-+";
  boost::scoped_ptr<test::TestConnection> con(new test::TestConnection(response));
  boost::scoped_ptr<GdbPacketHandler> handler(new GdbPacketHandler(*con));

  GdbPacket packet = CreateGdbPacket(packetContent);
  handler->WriteGdbPacket(packet);

  BOOST_REQUIRE_EQUAL(con->GetOutputWritten(), packetString);
}

BOOST_AUTO_TEST_SUITE_END()
