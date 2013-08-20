package np;

import java.util.Arrays;

/**Multi-step indexing object.
 *TODO: Extend beyond indexing to include slicing...probably call it "selector" or similar then
 */
public class Index {
  private final int[] indexes;
  public Index(int idx) {this.indexes = new int[]{idx};}
  public Index(int[] idxs) {this.indexes = Arrays.copyOf(idxs, idxs.length);}

  public Index then(int idx) {
    int[] nidx = new int[indexes.length+1];
    System.arraycopy(indexes,0,nidx,0,indexes.length-1);
    nidx[nidx.length-1] = idx;
    return new Index(nidx);
  }
}
