//per http://pommereau.blogspot.com/2009/05/interfacing-java-and-python-through-jni.html
public class JNI {
	private native String Hello(String name);

	public static void main(String[] args) {
		String text = new JNI().Hello("Python");
		System.out.println(text);
	}

	static {
		System.loadLibrary("helloworld");
	}
}
