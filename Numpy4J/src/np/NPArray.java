package np;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.ByteOrder;

public class NPArray {
  //NOTE:Fields are accessed BY NAME in JNI.  Name changes require JNI access changes as well
  private final ByteBuffer buffer; 
  private final NPType type;
  
  /**Address of any backing python resources.**/
  private final long pyaddr = 0;

  private NPArray(ByteBuffer buffer, NPType type){
    this.buffer = buffer;
    this.type = type;
  }

  protected void finalize() {
    if (pyaddr!=0) {JNIBridge.freePython(pyaddr);}
  }


  public NPType type() {return type;}
  public int size() {return buffer.capacity()/type.dtype().bytes;}
  
  //TODO: Modify the "buffer.getXXX" to respect dtype
  public Number getRaw(int i) {return buffer.getInt(i*type.dtype().bytes);}
  public int getRawInt(int i) {return buffer.getInt(i*type.dtype().bytes);}
  public Number getValue(Index i) {return null;}
  public NPArray getSlice(Index i) {return null;}

  //TODO: Extend to respect dtype 
  public void arange() {
    IntBuffer ints = buffer.asIntBuffer();
    for (int i=0; i<size(); i++) {ints.put(i,i);}
  }

  public static int DISPLAY_LIMIT=10;
  
  //TODO: Extend to consider dtype 
  public String toString() {
    StringBuilder b = new StringBuilder();
    b.append("NPArray[");
    b.append(size());
    b.append("]{");

    IntBuffer ints = buffer.asIntBuffer();
    for (int i=0; i<DISPLAY_LIMIT && i < size(); i++) {
      b.append(ints.get(i));
      b.append(", ");
    }
    b.deleteCharAt(b.length()-1);
    b.deleteCharAt(b.length()-1);
    if (size() > DISPLAY_LIMIT) {b.append("...");} 
    b.append("}");
    return b.toString();
  }

  public static NPArray allocate(NPType type, int size) {
    NPType.DTYPE dtype = type.dtype();
    if (dtype == NPType.DTYPE.DEFER) {throw new IllegalArgumentException("dtype still set as defer.  Cannot instantiate.");}

    int capacity = size * dtype.bytes;
    ByteBuffer b = ByteBuffer.allocateDirect(capacity);
    b.order(ByteOrder.nativeOrder());  //TODO: add byte-order support to dtype
    return new NPArray(b, type);
  }
}
