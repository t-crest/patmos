
#include <boost/test/unit_test.hpp>

#include "debug/GdbMessage.h"
#include "TestConnection.h"
#include "debug/GdbPacketHandler.h"

#include <boost/scoped_ptr.hpp>

using namespace patmos;

BOOST_AUTO_TEST_SUITE ( GdbMessageTests )

BOOST_AUTO_TEST_CASE( GetSupportedMessageCreatesCorrectMessage )
{
  const std::string packetContent = "qSupported";
  GdbMessagePtr message = GetGdbMessage(packetContent);
  BOOST_REQUIRE_EQUAL(message->GetMessageString(), packetContent);
}

BOOST_AUTO_TEST_CASE( HandleSupportedMessageProducesCorrectResponse )
{
  const std::string packetContent = "qSupported";
  const std::string expectedResponse = "$PacketSize=0#60";
  GdbMessagePtr message = GetGdbMessage(packetContent);
  boost::scoped_ptr<test::TestConnection> con(new test::TestConnection("+"));
  GdbPacketHandler packetHandler(*con);

  message->Handle(packetHandler);
  BOOST_REQUIRE_EQUAL(con->GetOutputWritten(), expectedResponse);
}

BOOST_AUTO_TEST_CASE( GetReasonMessageCreatesCorrectMessage )
{
  const std::string packetContent = "?";
  GdbMessagePtr message = GetGdbMessage(packetContent);
  BOOST_REQUIRE_EQUAL(message->GetMessageString(), packetContent);
}

BOOST_AUTO_TEST_CASE( HandleGetReasonMessageProducesCorrectResponse )
{
  const std::string packetContent = "?";
  const std::string expectedResponse = "$S05#b8";
  GdbMessagePtr message = GetGdbMessage(packetContent);
  boost::scoped_ptr<test::TestConnection> con(new test::TestConnection("+"));
  GdbPacketHandler packetHandler(*con);

  message->Handle(packetHandler);
  BOOST_REQUIRE_EQUAL(con->GetOutputWritten(), expectedResponse);
}

BOOST_AUTO_TEST_CASE( GetUnsupportedMessageCreatesCorrectMessage )
{
  const std::string packetContent = "some unsupported message";
  const std::string messageString = "Unsupported Message: some unsupported message";
  GdbMessagePtr message = GetGdbMessage(packetContent);
  BOOST_REQUIRE_EQUAL(message->GetMessageString(), messageString);
}

BOOST_AUTO_TEST_CASE( HandleUnsupportedMessageThrowsException )
{
  const std::string packetContent = "some unsupported message";
  GdbMessagePtr message = GetGdbMessage(packetContent);
  boost::scoped_ptr<GdbConnection> con(new test::TestConnection(""));
  GdbPacketHandler packetHandler(*con);
  BOOST_REQUIRE_THROW(message->Handle(packetHandler), 
      GdbUnsupportedMessageException);
}

BOOST_AUTO_TEST_SUITE_END()
