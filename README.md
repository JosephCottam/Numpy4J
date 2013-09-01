Numpy4J
=======

Interface Java with Numpy.  Try not to be reasonable.

What are you trying to accomplish? Get Java access to the highly tuned Numpy array
library.  

Why bother?  At least one reason is that there is no reliable pure-Java 
way to get vectorized performance for dense matrix operations. This is a
_big deal_(TM).  Our preliminary investigation shows that (even with
the JNI overhead) we can array-array multiplication several hundred times 
faster by using the numpy and JNI over a naive pure-java solution.  Another is
just to explore working with these two technologies at once.


How do you get the JVM to talk with Python libraries?  Its a combination of
nio, JNI and an embedded python interpreter.  By using nio's _direct_ byte buffer's,
we can (usually) avoid copying data as it moves in and out of the JVM.
Using memoryview and buffer objects on the Python avoids copying as the values
move back in.

What works:
If you make an NPArray (that's the Java class that represents a Numpy array), 
you can execute max, min, log and array-array multiply.  For log and multiply,
the resulting NPArray is actually backed by memory that numpy allocated for its results array. 
The DType, array-order (C/Fortran), and byte order are properly communicated 
from the python to the Java side 
(but non-default NPType values don't move from Java to Python consistently).

Build
------

Before you can build, a system.properties file needs to be created in the Numpy4J subdirectory 
(i.e., right next to the "build.xml" file).
The entries in system.properties are used by the c-compiler to properly do imports and linking.
This file has three key/value pairs:

* java.path= _location of the JNI headers for your system_. Can be found by locating jni.h on your system. 
* python.path=_location of the python version-specific headers_.  Can be found by locating Python.h on your system.
* python.version=_python version name_.  Something like "python2.7". Running "python --version" will probably give a good idea of what to put here.

With system.properties in place, in the Numpy4J directory, run "ant".

Unit tests are run with "ant test".
To also test it, run "ant exec".
To do a quick(ish) perofrmance comparison, run "ant perf".

Status
-------

Most recent tests show that (100 iterations, array sizes greater than 5,000 items and two different machines):

* Numpy4J Max v.s NPArray yeilding primtitives: Numpy4j is twice as slow 
* Numpy4J Max vs. NPArray yielding Numbers: Numpy4j is five times faster
* Numpy4J Log vs. NPArray primtives and double[] result: Numpy4j is a little less than twice as fast
* Numpy4J Log vs. NParray primtivites and NPArray result: Numpy4j is a little more than twice as fast
* Numpy4j multiply vs. NPArray primtives and NPArray result: Numpy4j is at least three times faster (small arrays) and can be several thousand times faster (large arrays)
