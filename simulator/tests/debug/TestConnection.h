
#include "debug/GdbConnection.h"

namespace patmos
{
 namespace test
 {
  class TestConnection : public GdbConnection
  {
  public:
    TestConnection(std::string inputString)
      : m_inputString(inputString),
        m_inputPos(0),
        m_outputWritten()
    {
    }

    void PutChar(char c) const
    {
      m_outputWritten << c;
    }

    char GetChar() const
    {
      if (m_inputPos < m_inputString.length())
        return m_inputString[m_inputPos++];
      else
        return 0;
    }

    void Write(const std::string &str) const
    {
      m_outputWritten << str;
    }

    std::string GetOutputWritten() const
    {
      return m_outputWritten.str();
    }

  private:
    std::string m_inputString;
    mutable int m_inputPos;
    mutable std::stringstream m_outputWritten;
  };
 }
}
