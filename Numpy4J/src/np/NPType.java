package np;

import java.nio.ByteOrder;

/**Represents all of the numpy dtype, shape and flags information**/
public class NPType {
  public enum RAWTYPE {int8(1), int16(2), int32(4), int64(8), float32(4), float64(8), DEFER(0);
    public final int bytes;
     RAWTYPE(int bytes) {this.bytes = bytes;}
  }
  
  public enum BYTE_ORDER {
    big(ByteOrder.BIG_ENDIAN,">"), little(ByteOrder.LITTLE_ENDIAN,"<"), NATIVE(ByteOrder.nativeOrder(), "=");
    public final ByteOrder nio;
    public final String symbol;
    BYTE_ORDER(ByteOrder nio, String symbol) {
      this.nio = nio;
      this.symbol = symbol;
    }
  }

  public enum ORDER {c,fortran,DEFER}

  private final RAWTYPE rawtype;
  private final ORDER order;
  private final BYTE_ORDER byteorder;

  /**Default NPType is int32 and c-order.**/
  public NPType() {this(RAWTYPE.int32);}
  public NPType(RAWTYPE rawtype) {this(rawtype, ORDER.c, BYTE_ORDER.NATIVE);}
  public NPType(RAWTYPE rawtype, ORDER order, BYTE_ORDER byteorder) {
    this.rawtype = rawtype;
    this.order = order;
    this.byteorder = byteorder;
  }

  public NPType rawtype(RAWTYPE type) {return new NPType(type, this.order, this.byteorder);}
  public RAWTYPE rawtype() {return rawtype;}

  public NPType order(ORDER order) {return new NPType(this.rawtype, order, this.byteorder);}
  public ORDER order() {return order;}

  public NPType byteorder(BYTE_ORDER byteorder) {return new NPType(this.rawtype, this.order, byteorder);}
  public BYTE_ORDER byteorder() {return byteorder;}

  public String toString() {return String.format("[%s, %s]", rawtype.toString(), byteorder.symbol);}

  /**NPType with values that can be dered set to deferred and others with sensible defaults.*/
  public static NPType defer() {return new NPType(RAWTYPE.DEFER, ORDER.DEFER, BYTE_ORDER.NATIVE);}
}
