package np.test;

import static org.junit.Assert.*;
import org.junit.Test;

import np.*;

public class TestNPArray {
  @Test
  public void arange() {
    NPType t = new  NPType(NPType.RAWTYPE.int32);
    NPArray array = NPArray.allocate(t, 1000);
    
    array.arange();
    for (int i=0; i<array.size(); i++) {
      assertEquals("(0,1) -- error at " + i, i, array.getFlatInt(i));
    }

    array.arange(1, 10);
    for (int i=0; i<array.size(); i++) {
      assertEquals("(1,10) -- error at " + i, (10*i)+1, array.getFlatInt(i));
    }

    array.arange(5, 12);
    for (int i=0; i<array.size(); i++) {
      assertEquals("(1,10) -- error at " + i, (12*i)+5, array.getFlatInt(i));
    }
    
    array.arange(-2, -3);
    for (int i=0; i<array.size(); i++) {
      assertEquals("(1,10) -- error at " + i, (-3*i)-2, array.getFlatInt(i));
    }


  }

}
