package np;

import java.nio.ByteBuffer;

public class NPArray {
  private final ByteBuffer buffer;
  private final NPType type;

  private NPArray(ByteBuffer buffer, NPType type){
    this.buffer = buffer;
    this.type = type;
  }

  public NPType type() {return type;}
  public ByteBuffer buffer() {return buffer;}
  public int size() {return buffer.capacity()/type.dtype().bytes;}

  public static NPArray allocate(NPType type, int size) {
    NPType.DTYPE dtype = type.dtype();
    if (dtype == NPType.DTYPE.DEFER) {throw new IllegalArgumentException("dtype still set as defer.  Cannot instantiate.");}

    int capacity = size * dtype.bytes;
    ByteBuffer b = ByteBuffer.allocateDirect(capacity);
    return new NPArray(b, type);
  }
}
