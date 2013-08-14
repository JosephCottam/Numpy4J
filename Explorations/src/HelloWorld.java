//per http://pommereau.blogspot.com/2009/05/interfacing-java-and-python-through-jni.html
public class HelloWorld {
	private native String Hello(String name);

	public static void main(String[] args) {
    String say = args.length > 0 ? args[0] : "Python";
    System.out.println("one");
		String text = new HelloWorld().Hello(say);
    System.out.println("two");
    System.out.println(text);
    System.out.println("three");
	}

	static {
    try {
		System.loadLibrary("helloworld");
    } catch (Exception e) {
      System.out.println("Error linking through jni...");
      e.printStackTrace();
    }
	}
}
