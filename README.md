php-avahi
=========

Avahi is a system which facilitates service discovery on a local network (eg. Rendezvous, Bonjour, Zeroconf) and is very convenient.

php-avahi is an extension that act as client to browse services.

It is highly inspired from the [php5-avahi](https://code.google.com/archive/p/php5-avahi/) project.

Dependencies
------------

You will need `php` and `avahi` installed.
For archlinux, use the following command:

```
# pacman -S php avahi
```

For ubuntu, use the following command:

```
# apt install php-dev libavahi-client-dev libavahi-common-dev
```

Compiling & installing
----------------------

As every PHP extension, it should be compiled using the following commands:

```
$ phpize
$ ./configure
$ make
# make install
```

Configure
---------

To enable it in php.ini, add the following line in the extension section:

```
...
extension=avahi
...
```

TODO
----

* Add service creation support (?)
* Add Rendezvous, Bonjour, Zeroconf support

Pull Requests
-------------

I don't intend to spend a lot of time on this. Feel free to send me PR, and I'll review them ASAP.
