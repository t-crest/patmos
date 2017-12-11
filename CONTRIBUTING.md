# Contributing to Patmos and T-CREST

Thank you for contributing to Patmos and T-CREST.

## Patmos Coding Style

Only ASCII characters in source code.

Scala is very flexible relative to file names and directory structure.
However, within the Patmos project we intend to adhere to the common
Java/Scala coding style.

Symbols and names are written in CamelCase. No C style _ shall be
used (_ has a special meaning in Scala).

Types (classes) start with an upper case character. Class members
(fields, methods), functions, and local variables start with a lower
case character. Constants are written all upper case.

Indentation is two spaces.

File names have the name of the (main) class. Helper classes can
be in the same file. If the file contains a collection of related
classes, but no main class (e.g., connections.scala) the file
name shall start with lower case.

Packages are organized in the Java style, where the directories
represent the package hierarchy.

It is important in HW design to quickly distinguish between combinational
circuits and registers. Therefore, all registers shall end with Reg
(e.g., cntReg).

Memory components, if in a larger context, should also be named with the
postfix Mem (e.g., tagMem).

For more info on common Scala coding conventions see:
http://docs.scala-lang.org/style/
