package np.test;

import np.*;

/**Target for the exec task.  This class holds a main method for quick-tests of features."**/
public class Exec {
  public static void main(String[] args){ 
    NPType type = new NPType();
    NPArray array = NPArray.allocate(type, 10);
    array.arange();
    JNIBridge bridge= new JNIBridge();

    System.out.printf("Input: %s\n", array);

    System.out.printf("Max: %s\n", bridge.max(array));
    System.out.printf("Min: %s\n", bridge.min(array));
    System.out.printf("Log: %s\n", bridge.log(array));
    System.out.printf("Mult: %s\n", bridge.mult(array, array));
  }
}
