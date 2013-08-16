package np;

/**Represents all of the numpy dtype, shape and flags information**/
public class NPType {
  enum DTYPE {int8(8), int16(16), int32(32), int64(64), float32(32), float64(64), DEFER(0);
    public final int bytes;
     DTYPE(int bytes) {this.bytes = bytes;}
  }

  enum ORDER {c,fortran,DEFER}

  private final DTYPE dtype;
  private final ORDER order;

  public NPType() {this(DTYPE.DEFER, ORDER.DEFER);}
  public NPType(DTYPE dtype, ORDER order) {
    this.dtype = dtype;
    this.order = order;
  }

  public NPType dtype(DTYPE type) {return new NPType(type, this.order);}
  public DTYPE dtype() {return dtype;}

  public NPType order(ORDER order) {return new NPType(this.dtype, order);}
  public ORDER order() {return order;}
}
