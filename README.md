Numpy4J
=======

Interface Java with Numpy.  Try not to suck.


Right now, you can make a java byte buffer, stuff it full of integers and get numpy to tell you the maximum value...and not much more.

Build
------

Before you can build, a system.properties file needs to be created in the Numpy4J directory.  
The entries in system.properties are used by the c-compiler to properly do imports and linking.
This file has three key/value pairs:

* java.path= _location of the JNI headers for your system_. Can be found by locating jni.h on your system. 
* python.path=_location of the python version-specific headers_.  Can be found by locating Python.h on your system.
* python.version=_python version name_.  Something like "python2.7". Running "python --version" will probably give a good idea of what to put here.

With system.properties in place, in the Numpy4J directory, run "ant".

To also test it, do "ant exec".

To build and then run "ant build exec".

