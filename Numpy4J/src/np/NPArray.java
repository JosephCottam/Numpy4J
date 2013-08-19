package np;

import java.nio.ByteBuffer;
import java.nio.IntBuffer;

public class NPArray {
  private final int test=1;
  private final ByteBuffer buffer; 
  private final NPType type;

  private NPArray(ByteBuffer buffer, NPType type){
    this.buffer = buffer;
    this.type = type;
  }

  public NPType type() {return type;}
  public int size() {return buffer.capacity()/type.dtype().bytes;}
  
  /**Actual backing buffer.  This method is accessed by name via JNI, do not change without updating JNI code.**/
  public ByteBuffer buffer() {return buffer;}

  public Number getRaw(int i) {return null;}
  public Number getValue(Index i) {return null;}
  public NPArray getSlice(Index i) {return null;}

  //TODO: Extend to respect dtype 
  public void arange() {
    IntBuffer ints = buffer.asIntBuffer();
    int size = ints.capacity()/type.dtype().bytes;
    for (int i=0; i<size; i++) {ints.put(i,i);}
  }

  public static int DISPLAY_LIMIT=10;
  //TODO: Extend to respect dtype 
  public String toString() {
    StringBuilder b = new StringBuilder();
    b.append("NPArray{");
    
    IntBuffer ints = buffer.asIntBuffer();
    int size = ints.capacity()/type.dtype().bytes;
    for (int i=0; i<DISPLAY_LIMIT && i < size; i++) {
      b.append(ints.get(i));
      b.append(", ");
    }
    b.deleteCharAt(b.length()-1);
    b.deleteCharAt(b.length()-1);
    if (size > DISPLAY_LIMIT) {b.append("...");} 
    b.append("}");
    return b.toString();
  }

  public static NPArray allocate(NPType type, int size) {
    NPType.DTYPE dtype = type.dtype();
    if (dtype == NPType.DTYPE.DEFER) {throw new IllegalArgumentException("dtype still set as defer.  Cannot instantiate.");}

    int capacity = size * dtype.bytes;
    ByteBuffer b = ByteBuffer.allocateDirect(capacity);
    return new NPArray(b, type);
  }
}
