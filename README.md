Numpy4J
=======

Interface Java with Numpy.  Try not to be reasonable.

What are you trying to accomplish? Get Java access to the highly tuned Numpy array
library.  

Why bother?  At least one reason is that there is no reliable pure-Java 
way to get vectorized performance for dense matrix operations. This is a
_big deal_(TM).  Our preliminary investigation shows that (even with
the JNI overhead) we can do array-array multiplication several hundred times 
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
To do a quick perofrmance comparison, run "ant perf".



Status
-------

Performance is one of the goals, so we are monitoring it already.  We have two major comarisons groups.
The first compares Numpy4J to using a ByteBuffer inside of Java.  The second compares Numpy4J to 
using Java arrays.  Our tests are Max, Log and Multiply.  For the sake of curiosity, we ran the "max" test
wit the ByteBuffer veresion returning Number objects and returning ints.  In general, if an array of values 
is returned, Numpy4J is eventually faster by a significant amount.

Results were acquired via the "ant perf" task.

Running with 100 iterations (let the JVM know we're serrious?) and array sizes betweeen 10 and 5.2 million:

* Max Numpy4J vs. primtive array:  Pritimite arrays register essentially zero time...Numpy4j always looses 
* Max Numpy4J vs. NPArray returning ints: Numpy4j is 80\% slower than pure java
* Max Numpy4J vs. NPArray returning Numbers: Numpy4j is five times faster
* Log Numpy4J vs. NPArray returning ints: Numpy4j is a little more than twice as fast
* Log Numpy4J vs. primtive array: Numpy4J is a little less than twice as fast
* Multiply Numpy4J vs. NPArray returning ints: Numpy4J is a little more than three times faster
* Multiply Numpy4J vs. primitive array: Numpy4J is a little less than twice as fast
