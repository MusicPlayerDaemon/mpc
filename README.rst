mpc
===

mpc is a command-line client for the `Music Player Daemon
<http://www.musicpd.org/>`__.


Installing mpc from source
--------------------------

You need:

- a C99 compliant compiler (e.g. gcc)
- libmpdclient 2.9

Run ``./configure`` (or ``./autogen.sh`` if you cloned from git)::

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


Links
-----

- `Home page and download <http://www.musicpd.org/clients/mpc/>`__
- `git repository <https://github.com/MusicPlayerDaemon/mpc/>`__
- `Bug tracker <https://github.com/MusicPlayerDaemon/mpc/issues>`__
- `Forum <http://forum.musicpd.org/>`__
