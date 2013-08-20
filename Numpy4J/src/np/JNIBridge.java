package np;

public class JNIBridge {
  static {System.loadLibrary("JNIBridge");}

  public native int min(NPArray array);
  public native int max(NPArray array);
//  public native Number min(NPArray array);
//  public native Number mean(NPArray array);


  public static void main(String[] args){ 
    NPType type = new NPType();
    NPArray array = NPArray.allocate(type, 40);
    array.arange();
    JNIBridge bridge= new JNIBridge();

    System.out.printf("Input: %s\n", array);

    System.out.printf("Max: %s\n", bridge.max(array));
    System.out.printf("Min: %s\n", bridge.min(array));

  }
}
