package np.test;

import np.*;

public class Tests {
  public static final JNIBridge BRIDGE= new JNIBridge();
  public static interface Action {
    public long probeBridge(NPArray array);
    public long probeJVM(NPArray array);
  }

  public static void searchCrossover(Action r) {
    int size=10000;
    int low=size;
    int high=size;
    boolean found=false;

    while (!found) {
      NPType type = new NPType();
      NPArray array = NPArray.allocate(type, size);
      array.arange();

      long before = System.currentTimeMillis();
      r.probeBridge(array);
      long after = System.currentTimeMillis();
      long bridgeTime = after-before;

      before = System.currentTimeMillis();
      r.probeJVM(array);
      after = System.currentTimeMillis();
      long jvmTime = after-before;
    }
  }

  /**Find where the bridge is more efficient than java specific tasks.**/
  public static void crossoverMax() {
    Action r = new Action() {
      public long probeBridge(NPArray array) {return BRIDGE.max(array);}
      public long probeJVM(NPArray array) {
        int max = Integer.MIN_VALUE;
        for (int i=0; i<array.size(); i++) {
          max = Math.max(max, array.getRaw(i).intValue());
        }
        return max;
      }
    };
    searchCrossover(r);

    
  }
 
  public static void main(String[] args){ 
    NPType type = new NPType();
    NPArray array = NPArray.allocate(type, 40);
    array.arange();
    JNIBridge bridge= new JNIBridge();

    System.out.printf("Input: %s\n", array);

    System.out.printf("Max: %s\n", bridge.max(array));
    System.out.printf("Min: %s\n", bridge.min(array));

  }
}
