
#include <boost/test/unit_test.hpp>

#include "debug/GdbMessageHandler.h"
#include "TestConnection.h"
#include "debug/GdbPacketHandler.h"
#include "debug/GdbMessage.h"

#include <boost/scoped_ptr.hpp>

using namespace patmos;

BOOST_AUTO_TEST_SUITE ( GdbMessageHandlerTests )

BOOST_AUTO_TEST_CASE( GetSupportedMessageReturnsCorrectMessage )
{
  const std::string messageString = "qSupported";
  const std::string packetString = "$qSupported#37";

  boost::scoped_ptr<test::TestConnection> 
    con(new test::TestConnection(packetString));
  GdbPacketHandler packetHandler(*con);
  GdbMessageHandler messageHandler(packetHandler);

  GdbMessagePtr message = messageHandler.ReadGdbMessage();

  BOOST_REQUIRE_EQUAL(message->GetMessageString(), messageString);
}

BOOST_AUTO_TEST_CASE( GetReasonMessageReturnsCorrectMessage )
{
  const std::string messageString = "?";
  const std::string packetString = "$?#3f";

  boost::scoped_ptr<test::TestConnection> 
    con(new test::TestConnection(packetString));
  GdbPacketHandler packetHandler(*con);
  GdbMessageHandler messageHandler(packetHandler);

  GdbMessagePtr message = messageHandler.ReadGdbMessage();

  BOOST_REQUIRE_EQUAL(message->GetMessageString(), messageString);
}

BOOST_AUTO_TEST_SUITE_END()
