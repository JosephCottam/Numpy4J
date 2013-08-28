package np;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;
import java.nio.ByteOrder;


//TODO: Add slice support, and a corresponding "getValue(indexer)" that gets a value out, traversing slicing
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
  
  /**Return the underlying byte buffer. 
   *
   * WARNING: This is a reference to the internal structure.  
   * Changes to values in the buffer are reflected in the original object.
   * This should only be done with caution.
   * (That being said, a lot good can be achieved by sharing the buffer.)
   */
  public ByteBuffer buffer() {return buffer;}

  /**How many items of the specified type are in this array?**/
  public int size() {return buffer.capacity()/type.rawtype().bytes;}

  /**Treat the buffer as a flat aray, get the i-th item.
   * Still respects dtype and byte-order.
   */
  public Number getFlat(int i) {
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
  public int getFlatInt(int i) {return buffer.getInt(i*type.rawtype().bytes);}
  public void setFlatDouble(int i, double d) {buffer.putDouble(i, d);}
  public void setFlatLong(int i, long d) {buffer.putLong(i, d);}

  //TODO: Extend to respect nptype 
  public void arange() {arange(0,1);}
  public void arange(int start, int step) {
    IntBuffer ints = buffer.asIntBuffer();
    int val = start;
    for (int i=0; i<size(); i++) {
      ints.put(i,val);
      val += step;
    }
  }

  public static int DISPLAY_LIMIT=10;
  
  public String toString() {
    StringBuilder b = new StringBuilder();
    b.append("NPArray[");
    b.append(size());
    b.append("]{");

    for (int i=0; i<DISPLAY_LIMIT && i < size(); i++) {
      b.append(getFlat(i).toString());
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
