package np.test;

import np.*;
import np.util.Args;

public class Tests {
  public static final JNIBridge BRIDGE= new JNIBridge();
  public static interface Action {
    public long probeBridge(NPArray array);
    public long probeJVM(NPArray array);
    public String name();
  }

  public static int ITERATIONS = 1000;
  public static void searchCrossover(Action r) {
    int size=10;

    while (size < Integer.MAX_VALUE) {
      System.out.printf("Testing at %d %d times\n", size, ITERATIONS);
      NPType type = new NPType();
      NPArray array = NPArray.allocate(type, size);
      array.arange();

      long before = System.nanoTime();
      long bridgeVal=0;;
      for (int i=0; i<ITERATIONS; i++) {bridgeVal = r.probeBridge(array);}
      long after = System.nanoTime();
      final long bridgeTime = after-before;

      before = System.nanoTime();
      long jvmVal=0;
      for (int i=0; i<ITERATIONS; i++) {jvmVal = r.probeJVM(array);}
      after = System.nanoTime();
      final long jvmTime = after-before;

      if (bridgeVal != jvmVal) {
        System.out.printf("Non-matching values %d (jvm) vs. %d (np) at size %d.\nABORTING!!!!\n\n", jvmVal, bridgeVal, size);
        break;
      }
      
      System.out.printf("count %d -- Bridge/JVM=Ratio  : %d/%d=%f\n", size, bridgeTime, jvmTime, bridgeTime/(double)jvmTime);
      if (bridgeTime < jvmTime) {
        System.out.printf("%s crossover: %d items\n", r.name(), size);
        break;
      }

      size = size*2;
    }

  }

  /**Find where the bridge is more efficient than java specific tasks.**/
  public static void crossoverMax() {
    Action r = new Action() {
      public long probeBridge(NPArray array) {return BRIDGE.max(array);}
      public long probeJVM(NPArray array) {
        int max = Integer.MIN_VALUE;
        for (int i=0; i<array.size(); i++) {
          max = Math.max(max, array.getRawInt(i));
        }
        return max;
      }
      public String name() {return "Max";}
    };
    searchCrossover(r);

    
  }
 
  public static void main(String[] args){ 
    ITERATIONS = Integer.parseInt(Args.key(args,"-i",Integer.toString(ITERATIONS)));
    crossoverMax();
  }
}
