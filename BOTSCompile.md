# Introduction #

This page is intended to explain how to compile the EQEmu source code to enable BOTS.


# Details #

BOTS can be compiled easily for both a Windows and a Linux server. The following are the instructions for what must be done to enable BOTS for either platform.

### Windows ###

  1. Open up Visual Studio and select the "ReleaseBots" configuration from the _Configuration Manager_ in the _Build_ menu.
  1. Compile it.

### Linux ###

  1. Edit your "makefile" in _.\zone_ and add "-DBOTS" to DFLAGS.
  1. Edit your "makefile.common" file in _.\zone_ and add both "bot.o" and "botspellsai.o"
  1. Edit your "makefile" in _.\world_ and add "-DBOTS" to DFLAGS.
  1. Compile it.

### Notes ###
If you are building a brand new database, then after you source the latest [PEQ](http://www.projecteq.net/) world database, be sure to also source the "bots.sql" file to install all the required BOTS database objects.