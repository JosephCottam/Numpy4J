package np.test;

import np.*;
import np.util.Args;

public class Performance {
  public static final JNIBridge BRIDGE= new JNIBridge();
  public static interface Action {
    public void probeBridge(NPArray array);
    public void probeJVMArray(int[] array);
    public void probeJVM(NPArray array);
    public String name();
  }

  public static int ITERATIONS = 100;
  public static int SIZE = 10;
  public static int FINAL_SIZE=10000000;

  public static void report(Action r) {
    int size=SIZE;

    System.out.printf("test, items, iters, avg bridge time (ms), avg NPArray time (ms), avg array time (ms), bridge/NPArray, bridge/array\n");

    String reportFormat = "%s, %d, %d, %.2f, %.2f, %.2f, %.2f, %.2f\n";

    while (size < FINAL_SIZE) {
      NPType type = new NPType();
      NPArray nparray = NPArray.allocate(type, size);
      nparray.arange(1,1);

      int[] nativearray = new int[nparray.buffer().asIntBuffer().limit()];
      for (int i=0; i<nativearray.length; i++) {
        nativearray[i] = nparray.getFlatInt(i);
      }

      long before = System.nanoTime();
      for (int i=0; i<ITERATIONS; i++) {r.probeBridge(nparray);}
      long after = System.nanoTime();
      final long bridgeTime = after-before;

      before = System.nanoTime();
      for (int i=0; i<ITERATIONS; i++) {r.probeJVM(nparray);}
      after = System.nanoTime();
      final long jvmNPTime = after-before;
      
      before = System.nanoTime();
      for (int i=0; i<ITERATIONS; i++) {r.probeJVMArray(nativearray);}
      after = System.nanoTime();
      final long jvmArrayTime = after-before;

      double avgbridgems = (bridgeTime/(double) ITERATIONS)/(double) 1000000;
      double avgNPms = (jvmNPTime/(double) ITERATIONS)/(double) 1000000;
      double avgArrayms = (jvmArrayTime/(double) ITERATIONS)/(double) 1000000;

      System.out.printf(reportFormat, r.name(), size, ITERATIONS, avgbridgems, avgNPms, avgArrayms, bridgeTime/(double)jvmNPTime, bridgeTime/(double) jvmArrayTime);
      size = size*2;
    }
  }

  public static void log() {
    Action r = new Action() {
      public void probeBridge(NPArray array) {BRIDGE.log(array);}
      
      public void probeJVMArray(int[] array) {
        double[] target = new double[array.length];
        for (int i=0; i<array.length; i++) {
          target[i] =Math.log(array[i]);
        }
      }

      public void probeJVM(NPArray array) {
        NPType t = array.type().rawtype(NPType.RAWTYPE.float64); //matches the default python...at least on my machine
        NPArray target = NPArray.allocate(t, array.size());
        for (int i=0; i<array.size(); i++) {
          target.setFlatDouble(i, Math.log(array.getFlatInt(i)));
        }
      }
      public String name() {return "Log (int->double)";}
    };
    
    report(r);
  }


  //Includes overhead of allocating a Number object..
  public static void maxNumber() {
    Action r = new Action() {
      public void probeBridge(NPArray array) {BRIDGE.max(array);}
      
      public void probeJVMArray(int[] array) {
        int max = Integer.MIN_VALUE;
        for (int i=0; i<array.length; i++) {
          max = Math.max(max, array[i]);
        }
      }

      public void probeJVM(NPArray array) {
        int max = Integer.MIN_VALUE;
        for (int i=0; i<array.size(); i++) {
          max = Math.max(max, array.getFlat(i).intValue());
        }
      }
      public String name() {return "Max (int->Number)";}
    };

    report(r);
  }


  public static void mult() {
    Action r = new Action() {
      public void probeBridge(NPArray array) {BRIDGE.mult(array,array);}
      
      public void probeJVMArray(int[] array) {
        long[] target = new long[array.length];
        
        for (int i=0; i<array.length; i++) {
          long v = array[i];
          target[i] = v;
        }
      }
     
      public void probeJVM(NPArray array) {
        NPType t = array.type().rawtype(NPType.RAWTYPE.int64);
        NPArray target = NPArray.allocate(t, array.size());
        
        for (int i=0; i<array.size(); i++) {
          long v = array.getFlatInt(i);
          array.setFlatLong(i, v);
        }
      }
      public String name() {return "Mult (int->long)";}
    };

    report(r);
  }

  //Gets the value out as an 'int'...but you need a method for each primitive type
  public static void maxInt() {
    Action r = new Action() {
      public void probeBridge(NPArray array) {BRIDGE.max(array);}
      
      public void probeJVMArray(int[] array) {
        int max = Integer.MIN_VALUE;
        for (int i=0; i<array.length; i++) {
          max = Math.max(max, array[i]);
        }
      }

      public void probeJVM(NPArray array) {
        int max = Integer.MIN_VALUE;
        for (int i=0; i<array.size(); i++) {
          max = Math.max(max, array.getFlatInt(i));
        }
      }
      public String name() {return "Max (int->int)";}
    };

    report(r);
  }
 
  public static void main(String[] args){ 
    ITERATIONS = Integer.parseInt(Args.key(args,"-i",Integer.toString(ITERATIONS)));
    SIZE = Integer.parseInt(Args.key(args,"-n",Integer.toString(SIZE)));
    FINAL_SIZE = Integer.parseInt(Args.key(args,"-fn",Integer.toString(FINAL_SIZE)));

    maxNumber();
    maxInt();
    log();
    mult();
  }
}
