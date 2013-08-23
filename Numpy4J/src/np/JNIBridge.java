package np;

public class JNIBridge {
  static {System.loadLibrary("JNIBridge");}

  public native int min(NPArray array);
  public native int max(NPArray array);
  public native NPArray log(NPArray array);
}
