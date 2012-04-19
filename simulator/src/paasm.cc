//
//  This file is part of the Patmos Simulator.
//  The Patmos Simulator is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  The Patmos Simulator is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with the Patmos Simulator. If not, see <http://www.gnu.org/licenses/>.
//
//
// Main file of a simple Patmos assembler.
//

#include "assembler.h"
#include "streams.h"

#include <boost/format.hpp>
#include <boost/spirit/include/karma_generate.hpp>
#include <boost/spirit/include/karma_binary.hpp>

#include <fstream>
#include <iostream>
#include <iterator>

int main(int argc, char **argv)
{
  std::string line;
  unsigned int num_lines = 0;
  unsigned int code_size = 0;
  unsigned int num_errors = 0;
  patmos::line_assembler_t paasm;

  // check arguments
  if (argc != 3)
  {
    std::cerr << "Usage: paasm <input> <output>\n";
    return 1;
  }

  // open streams
  std::istream &in = patmos::get_stream<std::ifstream>(argv[1], std::cin);
  std::ostream &out = patmos::get_stream<std::ofstream>(argv[2], std::cout);

  std::ostream_iterator<char> cout_iterator (out);

  while (!in.eof())
  {
    patmos::dword_t iw;

    std::getline(in, line);
    num_lines++;

    // skip comments and empty lines
    if (line.empty() || *line.begin() == '#')
    {
      continue;
    }

    // parse the assembly line
    if (!paasm.parse_line(line, iw))
    {
      std::cerr << boost::format("Invalid Patmos assembly line:\n%1%: '%2%'\n")
                % num_lines % line;

      num_errors++;
    }
    else
    {
      // emit binary code
      if (iw >= 0)
      {
        boost::spirit::karma::generate(cout_iterator,
                                  boost::spirit::big_dword((patmos::word_t)iw));
        code_size += sizeof(patmos::word_t);
      }
      else
      {
        boost::spirit::karma::generate(cout_iterator,
                                       boost::spirit::big_qword(iw));
        code_size += sizeof(patmos::dword_t);
      }
    }
  }

  // some status messages
  std::cerr << boost::format("Emitted: %1% words\nErrors : %2%\n")
            % code_size % num_errors;

  // free streams
  patmos::free_stream(in, std::cin);
  patmos::free_stream(out, std::cout);

  return 0;
}
