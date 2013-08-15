//per http://pommereau.blogspot.com/2009/05/interfacing-java-and-python-through-jni.html
public class HelloWorld {
	private native String Hello(String name);

	public static void main(String[] args) {
    String say = args.length > 0 ? args[0] : "Python";
		String text = new HelloWorld().Hello(say);
    System.out.println(text);
	}

	static {
		System.loadLibrary("helloworld");
	}
}
