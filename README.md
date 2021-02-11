# libirc
C++ IRC library that is very versatile and designed for pretty much any type of IRC client (or server). libirc is a core component of GrumpyChat IRC system, which is a full featured IRC client with distributed core. You can see how it's implemented in its source code here: https://github.com/grumpy-irc/grumpy

# Design
libirc consist of 2 parts right now, the architecture supports server implementation as well, which is not however implemented yet.

## libirc
The core foundation that contains IRC definitions and objects that are shared between server and client implementations

## libircclient
IRC client foundations based on top of libirc, it overrides the basic definitions of IRC objects (users, channels, networks) and implements the client functionality into them. Every single aspect of libirc can be overriden and modified.

# Serialization
Because GrumpyChat requires ability to transfer C++ objects via network protocols in order to achieve its distributed model, libirc implements primitive ability to serialize and deserialize its objects. Each object is inherited from special class "Serializable" which implements two virtual functions:

* void LoadHash(QHash<QString, QVariant> hash)
* QHash<QString, QVariant> ToHash()

These two functions allow to turn whole C++ object into a QHash, which can be later either stored to disk, or serialized and sent over network. The hash can be user to instantiate these classes, either create new instance of class and then call LoadHash, or simply pass the hash into constructor of the class.

# Compiling libirc
If you have CMake and Qt on your system, you can compile libirc this way:
```bash
mkdir release
cd release
cmake ..
make

# If compiling with Qt5 use this switch
cmake .. -DQT5_BUILD=true

# NOTE: if you don't have Qt system-wide you can specify its install path using this
cmake .. -DCMAKE_PREFIX_PATH:PATH=~/Qt/5.15.2/clang_64/ -DQT5_BUILD=true
```
