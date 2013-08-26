package np;

public class JNIBridge {
  static {System.loadLibrary("JNIBridge");}

  public static native int min(NPArray array);
  public static native int max(NPArray array);
  public static native NPArray log(NPArray array);
  public static native NPArray mult(NPArray a1, NPArray a2);
  public static native void freePython(long resource);
}
