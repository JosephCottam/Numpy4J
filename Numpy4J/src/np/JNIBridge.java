package np;

public class JNIBridge {
  static {System.loadLibrary("JNIBridge");}

  public native int max(NPArray array);
//  public native Number min(NPArray array);
//  public native Number mean(NPArray array);


  public static void main(String[] args){ 
    NPType type = new NPType();
    NPArray array = NPArray.allocate(type, 10);
    JNIBridge bridge= new JNIBridge();

    System.out.println(bridge.max(array));

  }
}
