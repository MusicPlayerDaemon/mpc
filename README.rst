mpc
===

mpc is a client for the `Music Player Daemon
<http://www.musicpd.org/>`__. mpc provides a command line
user interface for controlling an MPD server running on the local network.

For more information about MPD and other clients visit http://www.musicpd.org/

mpc is released under the GPLv2. For the full license text, please see the
`COPYING <COPYING>`__ file for more detailed information.


Installing mpc from source
--------------------------

You need:

- the mpc source code: https://www.musicpd.org/clients/mpc/
- a C99 compliant compiler (e.g. gcc)
- libmpdclient 2.9

Run "./configure" (or "./autogen.sh" if you cloned from git)::

 ./configure

The configure option ``--help`` lists all available compile-time
options.

Compile and install::

 make
 make install


Using mpc
---------

Read mpc's manual page for usage instructions.


Bash-completion
---------------

If you want to be able to tab-complete the commands for mpc, you can copy the
contents of mpc-bashrc to your own ``~/.bashrc``.
