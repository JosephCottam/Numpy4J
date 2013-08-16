public class HelloWorld {
  private native String Hello(String name);
  public static void main(String[] args) {
    String text = new HelloWorld().Hello("Python");
    System.out.println(text);
  }
  static {
    System.loadLibrary("helloworld");
  }
}
