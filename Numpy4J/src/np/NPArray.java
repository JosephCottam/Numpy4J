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


  /**Create a NPArray backed by the given buffer, interpreted with the given type.
   *
   * Because byte order is a stateful part of the buffer, if the type's byte order
   * doesn't match the actual buffer byte order, a new buffer-view will be created 
   * and set to match the type's byte order.  The buffer view DOES NOT duplicate the
   * backing bytes, just the header information.
   */
  private NPArray(ByteBuffer buffer, NPType type){
    if (buffer.order() != type.byteorder().nio) {
      buffer = buffer.duplicate();
      buffer.order(type.byteorder().nio);
    }
    this.type = type;
    this.buffer = buffer;
  }

  protected void finalize() {
    if (pyaddr!=0) {JNIBridge.freePython(pyaddr);}
  }


  public NPType type() {return type;}
  public int size() {return buffer.capacity()/type.rawtype().bytes;}
  
  //TODO: Modify the "buffer.getXXX" to respect nptype
  public Number getRaw(int i) {
    int bytes = type.rawtype().bytes;
    switch(type.rawtype()) {
      case int8: return buffer.get(i);
      case int16: return buffer.getShort(i*bytes);
      case int32: return buffer.getInt(i*bytes);
      case int64: return buffer.getLong(i*bytes);
      case float32: return buffer.getFloat(i*bytes);
      case float64: return buffer.getDouble(i*bytes);
      default: throw new IllegalArgumentException("Unsupported raw return type: " + type.rawtype()); 
    }
    
  }
  public int getRawInt(int i) {return buffer.getInt(i*type.rawtype().bytes);}
  public Number getValue(Index i) {return null;}
  public NPArray getSlice(Index i) {return null;}

  //TODO: Extend to respect nptype 
  public void arange() {
    IntBuffer ints = buffer.asIntBuffer();
    for (int i=0; i<size(); i++) {ints.put(i,i);}
  }

  public static int DISPLAY_LIMIT=10;
  
  public String toString() {
    StringBuilder b = new StringBuilder();
    b.append("NPArray[");
    b.append(size());
    b.append("]{");

    for (int i=0; i<DISPLAY_LIMIT && i < size(); i++) {
      b.append(getRaw(i).toString());
      b.append(", ");
    }
    b.deleteCharAt(b.length()-1);
    b.deleteCharAt(b.length()-1);
    if (size() > DISPLAY_LIMIT) {b.append("...");} 
    b.append("}");
    return b.toString();
  }

  public static NPArray allocate(NPType type, int size) {
    NPType.RAWTYPE rawtype = type.rawtype();
    if (rawtype == NPType.RAWTYPE.DEFER) {throw new IllegalArgumentException("dtype still set as defer.  Cannot instantiate.");}

    int capacity = size * rawtype.bytes;
    ByteBuffer b = ByteBuffer.allocateDirect(capacity);
    b.order(ByteOrder.nativeOrder());  //TODO: add byte-order support to nptype
    return new NPArray(b, type);
  }
}
