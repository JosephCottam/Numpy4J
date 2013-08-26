package np.test;

import np.*;
import np.util.Args;

public class Performance {
  public static final JNIBridge BRIDGE= new JNIBridge();
  public static interface Action {
    public void probeBridge(NPArray array);
    public void probeJVM(NPArray array);
    public String name();
  }

  public static int ITERATIONS = 100;
  public static int SIZE = 10;
  public static int FINAL_SIZE=1000000;

  public static void report(Action r) {
    int size=SIZE;

    System.out.printf("test, items, iters, avg bridge time (ms), avg jvm time (ms), bridge/jvm\n");

    String reportFormat = "%s, %d, %d, %.2f, %.2f, %.2f\n";

    while (size < FINAL_SIZE) {
      NPType type = new NPType();
      NPArray array = NPArray.allocate(type, size);
      array.arange(1,1);

      long before = System.nanoTime();
      for (int i=0; i<ITERATIONS; i++) {r.probeBridge(array);}
      long after = System.nanoTime();
      final long bridgeTime = after-before;

      before = System.nanoTime();
      for (int i=0; i<ITERATIONS; i++) {r.probeJVM(array);}
      after = System.nanoTime();
      final long jvmTime = after-before;

      double avgbridgems = (bridgeTime/(double) ITERATIONS)/(double) 1000000;
      double avgJVMms = (jvmTime/(double) ITERATIONS)/(double) 1000000;

      System.out.printf(reportFormat, r.name(), size, ITERATIONS, avgbridgems, avgJVMms, bridgeTime/(double)jvmTime);
      size = size*2;
    }
  }

  public static void logArray() {
    Action r = new Action() {
      public void probeBridge(NPArray array) {BRIDGE.log(array);}
      public void probeJVM(NPArray array) {
        double[] rslt = new double[array.size()];
        for (int i=0; i<array.size(); i++) {
          rslt[i]=Math.log(array.getFlatInt(i));
        }
      }
      public String name() {return "Log (int->double, double[])";}
    };
    
    report(r);
  }
  public static void log() {
    Action r = new Action() {
      public void probeBridge(NPArray array) {BRIDGE.log(array);}
      public void probeJVM(NPArray array) {
        NPType t = array.type().rawtype(NPType.RAWTYPE.float64); //matches the default python...at least on my machine
        NPArray target = NPArray.allocate(t, array.size());
        for (int i=0; i<array.size(); i++) {
          target.setFlatDouble(i, Math.log(array.getFlatInt(i)));
        }
      }
      public String name() {return "Log (int->double, NPArray)";}
    };
    
    report(r);
  }


  //Includes overhead of allocating a Number object..
  public static void maxNumber() {
    Action r = new Action() {
      public void probeBridge(NPArray array) {BRIDGE.max(array);}
      public void probeJVM(NPArray array) {
        int max = Integer.MIN_VALUE;
        for (int i=0; i<array.size(); i++) {
          max = Math.max(max, array.getFlat(i).intValue());
        }
      }
      public String name() {return "Max (Number)";}
    };

    report(r);
  }


  //Gets the value out as an 'int'...but you need a method for each primitive type
  public static void maxInt() {
    Action r = new Action() {
      public void probeBridge(NPArray array) {BRIDGE.max(array);}
      public void probeJVM(NPArray array) {
        int max = Integer.MIN_VALUE;
        for (int i=0; i<array.size(); i++) {
          max = Math.max(max, array.getFlatInt(i));
        }
      }
      public String name() {return "Max (int)";}
    };

    report(r);
  }
 
  public static void main(String[] args){ 
    ITERATIONS = Integer.parseInt(Args.key(args,"-i",Integer.toString(ITERATIONS)));
    SIZE = Integer.parseInt(Args.key(args,"-n",Integer.toString(SIZE)));
    FINAL_SIZE = Integer.parseInt(Args.key(args,"-fn",Integer.toString(FINAL_SIZE)));

    //log();
    logArray();
    maxNumber();
    maxInt();
  }
}
