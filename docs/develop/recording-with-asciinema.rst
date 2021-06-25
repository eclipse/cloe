Recording with Asciinema
========================

`Asciinema <https://asciinema.org>`__ is a wonderful tool to record and replay
terminal sessions. This guide will help you make high-quality recordings for
the Cloe documentation.

.. warning::
   Asciinema provides the well-intentioned *option* of uploading your recording
   to `a public gallery <https://asciinema.org/explore/featured>`__.
   **This is nice but is problematic in a corporate context.**

   See the Configuration section below for a mitigation against this.

Installation
------------

Installing Asciinema itself on Ubuntu is a breeze::

   sudo apt install asciinema

In order to embed the recordings on this Sphinx site we need to install
the Sphinx `plugin <https://github.com/divi255/sphinxcontrib.asciinema>`__ as
well. There are several ways to go about this, but the following should get the
job done::

   pip3 install --user sphinxcontrib.asciinema

Configuration
-------------

**Before using Asciinema for anything, you should install the following
configuration file to** ``$HOME/.config/asciinema/config``::

   [api]
   url = http://localhost/disable-asciinema-upload

Run the following command to automate this::

   mkdir -p $HOME/.config/asciinema && echo -e "[api]\nurl = http://localhost/disable-asciinema-upload" > $HOME/.config/asciinema/config

Basic Usage
-----------

Once you have the mandatory configuration part done, you can enjoy the fruits
of your labor::

   asciinema rec ~/demo.asc

The above command will start recording the current session. You can pause with
[Ctrl+\] and end it with [Ctrl+D].

When you are done recording, you can replay the recording with::

   asciinema play ~/demo.asc

If you have a look at the options, you will see that you can also control
things like the playback speed and the idle time. Use [Space] and [Period] to
pause and step through the recording.

Recording Guidelines
--------------------

When recording a session for other people, there are a few things that are
recommended you watch out for. This is mandatory for all recordings that end
up on in the documentation.

1. **Use a terminal size of 100x25 characters.**
   The window used to play back a session must be at least as large as the
   window used to record the session. When embedded in the Sphinx
   documentation, this is especially important. If you have to, you may make
   the terminal window taller, up to 100 lines.

2. **Obfuscate your username and machine.**
   The recording is text-based and can be edited with your favorite editor.
   Alternatively, make use of sed or other search-and-replace programs.
   Change your username to something of equal length and the hostname too.
   Initial recommendation is ``captain`` as user and ``cucumber`` as host.

3. **Don't publish mistakes.**
   If there are mistakes in your recording, consider either removing them or
   re-recording.

4. **Make use of preload and idle-time-limit options.**
   When embedding in Sphinx, make use the ``:idle-time-limit:`` option to
   limit the delay between updates and preload the data so the user is
   tempted to press play::

      .. asciinema:: path_to_file.asc
         :preload: 1
         :idle-time-limit: 1
         :speed: 1

   See the `asciinema-player <https://github.com/asciinema/asciinema-player#asciinema-player-element-attributes>`__
   page on Github for more options.
