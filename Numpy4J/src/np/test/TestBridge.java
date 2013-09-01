package np.test;

import static org.junit.Assert.*;
import org.junit.Test;

import np.*;

public class TestBridge {
  
  @Test 
  public void multiply()  {
    NPType t = new  NPType(NPType.RAWTYPE.int32);
    NPArray array = NPArray.allocate(t, 10);
    array.arange();
    NPArray result = JNIBridge.mult(array,array);

    assertEquals(array.size(), result.size());
    for (int i=0; i<array.size(); i++) {
      assertEquals("Error at position " + i, i*i, result.getFlatInt(i));
    }
  }

  @Test
  public void max() {
    NPType t = new  NPType(NPType.RAWTYPE.int32);

    for (int i=10; i<1000000; i+=5423) {
      NPArray array = NPArray.allocate(t, i);
      array.arange();
      long max = JNIBridge.max(array);
      assertEquals(max, array.size()-1);
    }
  }
  
  @Test
  public void min() {
    NPType t = new  NPType(NPType.RAWTYPE.int32);

    for (int i=10; i<1000000; i+=2345) {
      NPArray array = NPArray.allocate(t, i);
      array.arange(10, -1);
      long min = JNIBridge.min(array);
      assertEquals(min, -(array.size()-11));
    }
  }

  @Test
  public void log() {
    NPType t = new NPType(NPType.RAWTYPE.int32);

    NPArray array = NPArray.allocate(t,3000);
    array.arange(1,3);
    NPArray rslt = JNIBridge.log(array);

    assertEquals("Result size unequal to input size.", array.size(), rslt.size());
    for (int i =0; i<array.size(); i++) {
      assertEquals("Mismatch at " + i, Math.log(array.getFlatInt(i)), rslt.getFlatDouble(i), .0001);
    }

  }
}
