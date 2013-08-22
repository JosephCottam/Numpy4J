package np.test;

import np.*;
import np.util.Args;

public class Performance {
  public static final JNIBridge BRIDGE= new JNIBridge();
  public static interface Action {
    public long probeBridge(NPArray array);
    public long probeJVM(NPArray array);
    public String name();
  }

  public static int ITERATIONS = 10;
  public static int SIZE = 10;
  public static int FINAL_SIZE=1000000;
  public static int FINAL_ITERATIONS=10000;

  public static void report(Action r) {
    int size=SIZE;
    int iterations=ITERATIONS;

    System.out.printf("items, iters, avg bridge time (ms), avg jvm time (ms), bridge/jvm\n");

    int iterWidth = Integer.toString(FINAL_ITERATIONS).length();
    String reportFormat = "%d, %" + iterWidth + "d, %.2f, %.2f, %.2f\n";

    while (size < FINAL_SIZE) {
      while (iterations < FINAL_ITERATIONS) {
        NPType type = new NPType();
        NPArray array = NPArray.allocate(type, size);
        array.arange();

        long before = System.nanoTime();
        long bridgeVal=0;;
        for (int i=0; i<iterations; i++) {bridgeVal = r.probeBridge(array);}
        long after = System.nanoTime();
        final long bridgeTime = after-before;

        before = System.nanoTime();
        long jvmVal=0;
        for (int i=0; i<iterations; i++) {jvmVal = r.probeJVM(array);}
        after = System.nanoTime();
        final long jvmTime = after-before;

        if (bridgeVal != jvmVal) {
          System.out.printf("Non-matching values %d (jvm) vs. %d (np) at size %d.\nABORTING!!!!\n\n", jvmVal, bridgeVal, size);
          break;
        }

        double avgbridgems = (bridgeTime/(double) iterations)/(double) 1000000;
        double avgJVMms = (jvmTime/(double) iterations)/(double) 1000000;

        System.out.printf(reportFormat, size, iterations, avgbridgems, avgJVMms, bridgeTime/(double)jvmTime);
        iterations=iterations*10;
      }
      iterations=ITERATIONS;
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
    report(r);

    
  }
 
  public static void main(String[] args){ 
    ITERATIONS = Integer.parseInt(Args.key(args,"-i",Integer.toString(ITERATIONS)));
    SIZE = Integer.parseInt(Args.key(args,"-n",Integer.toString(SIZE)));
    FINAL_ITERATIONS = Integer.parseInt(Args.key(args,"-fi",Integer.toString(FINAL_ITERATIONS)));
    FINAL_SIZE = Integer.parseInt(Args.key(args,"-fn",Integer.toString(FINAL_SIZE)));

    crossoverMax();
  }
}
