public class HelloCombined {
   static {
      System.loadLibrary("combinedJNI"); // hello.dll (Windows) or libhello.so (Unixes)
   }
   // A native method that receives nothing and returns void
   private native void sayHello(String name);
 
   public static void main(String[] args) {
     String name = args.length >0 ? args[0] : "World";
      new HelloCombined().sayHello(name);  // invoke the native method
   }
}
