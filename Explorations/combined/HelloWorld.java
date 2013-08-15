public class HelloWorld {

  private native String hello(String name);
  
  public static void main(String[] args) {
    String text = new HelloWorld().hello("Python");
    System.out.println(text);
  }
  static {
    System.loadLibrary("helloworld");
  }
}
