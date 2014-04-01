Debian package for the [Silicon Dust][1] [HDHomeRun][2] [DVB driver][3] written by [Villy Thomsen][4].

*I will be extending the driver to pass singal strength and symbol
quality information from the hdhomerun library.*

Only package sources are provided.  You must build the DEB's yourself.  It's
not hard, but I highly recommend you read the [Debian Maintainers build guide][5].

    $ git clone https://github.com/h0tw1r3/dvbhdhomerun
    $ cd dvbhdhomerun && dpkg-buildpackage -b
    $ cd ..
    $ dpkg -i <builtpackages>.deb

[1]: http://silicomdust.com/
[2]: http://www.silicondust.com/support/hdhomerun/downloads/linux/
[3]: http://sourceforge.net/projects/dvbhdhomerun/
[4]: mailto:tfylliv@gmail.com
[5]: http://www.debian.org/doc/manuals/maint-guide/build.en.html
