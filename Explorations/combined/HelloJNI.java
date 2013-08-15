public class HelloJNI {
   static {
      System.loadLibrary("helloJNI"); // hello.dll (Windows) or libhello.so (Unixes)
   }
   // A native method that receives nothing and returns void
   private native void sayHello();
 
   public static void main(String[] args) {
      new HelloJNI().sayHello();  // invoke the native method
   }
}
