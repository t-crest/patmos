# fastsd
This directory contains the software part for the fast SD controller, developed by Philipp Birkl as well as Martin Schwendinger for the course `Advanced Computer Architecture` at Technical Univerity of Vienna.

## Structure
The code is structured in logical blocks.
- `fatfs/` containes the implementation of FAT16/32 as well as vFAT written by [elm-chan][1].
- `sdclib/` containes the interface between the driver and the hardware.
- The root folder contains a simple demo application.
- Possibly a library `newlibfs-adapter` will be implemented to override the stub syscalls provided by newlib to get POSIX compatability. This however will conflict with the already given implementations in [patmos-newlib][2] as it is currently used to handle `STDOUT`.

## References
[1]: http://elm-chan.org/
[2]: https://github.com/t-crest/patmos-newlib/blob/4c149a53f8cb2478d99aac731b61b5e4ed63543f/libgloss/patmos/write.c#L41
