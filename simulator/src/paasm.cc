/*
   Copyright 2012 Technical University of Denmark, DTU Compute.
   All rights reserved.

   This file is part of the Patmos simulator.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

//
// Main file of a simple Patmos assembler.
//

#include "assembler.h"
#include "streams.h"

#include <boost/format.hpp>

#include <string>
#include <fstream>
#include <iostream>

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
  try
  {
    std::istream &in = *patmos::get_stream<std::ifstream>(argv[1], std::cin);
    std::ostream &out = *patmos::get_stream<std::ofstream>(argv[2], std::cout);

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

      // strip comments
      size_t pos = line.find("#");
      if (pos != std::string::npos) {
        line = line.substr(0, pos);
      }

      // replace tabs with spaces to make error outputs match
      // TODO ugly, but sufficient for now, and I want to minimize boost deps
      for (unsigned int i = 0; i < line.size(); i++) {
        if (line[i] != '\t') continue;
        line = line.replace(i, 1, 8, ' ');
        i += 7;
      }

      // parse the assembly line
      if (!paasm.parse_line(line, iw))
      {
        std::cerr << boost::format("Invalid Patmos assembly line:\n%1$5d: '%2%'\n")
                  % num_lines % line;
        paasm.print_error(std::cerr, 8);

        num_errors++;
      }
    }

    // emit program
    if (!paasm.write_program(out, code_size))
    {
      num_errors++;
    }

    // some status messages
    std::cerr << boost::format("Emitted: %1% bytes\nErrors : %2%\n")
              % (code_size * 4) % num_errors;

    // free streams
    patmos::free_stream(&in);
    patmos::free_stream(&out);
  }
  catch(std::ios_base::failure f)
  {
    std::cerr << f.what() << "\n";
    return -1;
  }

  return num_errors;
}
