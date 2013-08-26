package np;

import java.nio.ByteOrder;

/**Represents all of the numpy dtype, shape and flags information**/
public class NPType {
  public enum RAWTYPE {int8(1), int16(2), int32(4), int64(8), float32(4), float64(8), DEFER(0);
    public final int bytes;
     RAWTYPE(int bytes) {this.bytes = bytes;}
  }
  
  enum BYTE_ORDER {
    big(ByteOrder.BIG_ENDIAN), little(ByteOrder.LITTLE_ENDIAN), NATIVE(ByteOrder.nativeOrder());
    public final ByteOrder nio;
    BYTE_ORDER(ByteOrder nio) {this.nio = nio;}
  }

  enum ORDER {c,fortran,DEFER}

  private final RAWTYPE dtype;
  private final ORDER order;
  private final BYTE_ORDER byteorder;

  /**Default NPType is int32 and c-order.**/
  public NPType() {this(RAWTYPE.int32, ORDER.c, BYTE_ORDER.NATIVE);}
  public NPType(RAWTYPE dtype, ORDER order, BYTE_ORDER byteorder) {
    this.dtype = dtype;
    this.order = order;
    this.byteorder = byteorder;
  }

  public NPType rawtype(RAWTYPE type) {return new NPType(type, this.order, this.byteorder);}
  public RAWTYPE rawtype() {return dtype;}

  public NPType order(ORDER order) {return new NPType(this.dtype, order, this.byteorder);}
  public ORDER order() {return order;}

  public NPType byteOrder(BYTE_ORDER byteorder) {return new NPType(this.dtype, this.order, byteorder);}
  public BYTE_ORDER byteorder() {return byteorder;}

  /**NPType with both dtype and order defered.**/
  public static NPType defer() {return new NPType(RAWTYPE.DEFER, ORDER.DEFER, BYTE_ORDER.NATIVE);}
}
