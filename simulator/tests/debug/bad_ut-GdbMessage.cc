
#include <boost/test/unit_test.hpp>

#include "debug/GdbMessage.h"
#include "TestConnection.h"
#include "debug/GdbPacketHandler.h"
#include "debug/GdbMessageHandler.h"

#include <boost/scoped_ptr.hpp>

using namespace patmos;

BOOST_AUTO_TEST_SUITE ( GdbMessageTests )

BOOST_AUTO_TEST_CASE( SupportedMessageCreatesCorrectMessage )
{
  const std::string expectedMessageString = "qSupported";

  GdbMessage message;
  
  BOOST_REQUIRE_EQUAL(message.GetMessageString(), expectedMessageString);
}

BOOST_AUTO_TEST_CASE( HandleSupportedMessageProducesCorrectResponse )
{
  const std::string expectedResponse = "$PacketSize=200#c2";
  bool targetContinue;

  boost::scoped_ptr<test::TestConnection> con(new test::TestConnection("+"));
  GdbPacketHandler packetHandler(*con);
  GdbMessageHandler messageHandler(packetHandler);

  GdbMessage message;

  message.Handle(messageHandler, targetContinue);
  BOOST_REQUIRE_EQUAL(con->GetOutputWritten(), expectedResponse);
}

BOOST_AUTO_TEST_CASE( GetReasonMessageCreatesCorrectMessage )
{
  const std::string expectedMessageString = "?";
  
  GdbMessage message;
  
  BOOST_REQUIRE_EQUAL(message.GetMessageString(), expectedMessageString);
}

BOOST_AUTO_TEST_CASE( HandleGetReasonMessageProducesCorrectResponse )
{
  const std::string expectedResponse = "$S05#b8";
  bool targetContinue;
  
  boost::scoped_ptr<test::TestConnection> con(new test::TestConnection("+"));
  GdbPacketHandler packetHandler(*con);
  GdbMessageHandler messageHandler(packetHandler);

  GdbMessage message;

  message.Handle(messageHandler, targetContinue);
  BOOST_REQUIRE_EQUAL(con->GetOutputWritten(), expectedResponse);
}

BOOST_AUTO_TEST_CASE( UnsupportedMessageCreatesCorrectMessage )
{
  const std::string packetContent = "some unsupported message";
  const std::string messageString = "Unsupported Message: some unsupported message";
  
  GdbMessage message(packetContent);
  
  BOOST_REQUIRE_EQUAL(message.GetMessageString(), messageString);
}

BOOST_AUTO_TEST_CASE( HandleUnsupportedMessageThrowsException )
{
  const std::string packetContent = "some unsupported message";
  bool targetContinue;
  
  boost::scoped_ptr<test::TestConnection> con(new test::TestConnection("+"));
  GdbPacketHandler packetHandler(*con);
  GdbMessageHandler messageHandler(packetHandler);
  
  GdbMessage message(packetContent);
  
  BOOST_REQUIRE_THROW(message.Handle(messageHandler, targetContinue), 
      GdbUnsupportedMessageException);
}

BOOST_AUTO_TEST_SUITE_END()
