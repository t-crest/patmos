# Appliations for Patmos and T-CREST

Small example programs (single file) live in folder c.
Larger applications shall live in their own folder under
apps and provide their own Makefile for a build.

This Makefile is invoked from the main Makefile as follows:

```
make app APP=appname
```

There is a naming convention that the .elf file has the
name of the app. It will be copied by the main Makefile
into the tmp folder.

For an example see the hello app, which can be built
and downloaded with:

```
make app config download APP=hello
```
