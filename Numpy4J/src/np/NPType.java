package np;

/**Represents all of the numpy dtype, shape and flags information**/
public class NPType {
  public enum DTYPE {int8(1), int16(2), int32(4), int64(8), float32(4), float64(8), DEFER(0);
    public final int bytes;
     DTYPE(int bytes) {this.bytes = bytes;}
  }

  enum ORDER {c,fortran,DEFER}

  private final DTYPE dtype;
  private final ORDER order;

  /**Default NPType is int32 and c-order.**/
  public NPType() {this(DTYPE.int32, ORDER.c);}
  public NPType(DTYPE dtype) {this(dtype, ORDER.c);}
  public NPType(DTYPE dtype, ORDER order) {
    this.dtype = dtype;
    this.order = order;
  }

  public NPType dtype(DTYPE type) {return new NPType(type, this.order);}
  public DTYPE dtype() {return dtype;}

  public NPType order(ORDER order) {return new NPType(this.dtype, order);}
  public ORDER order() {return order;}

  /**NPType with both dtype and order defered.**/
  public static NPType defer() {return new NPType(DTYPE.DEFER, ORDER.DEFER);}
}
