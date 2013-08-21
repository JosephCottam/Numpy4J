package np.util;

public final class Args {
  private Args() {}
  
  /**Check for a prop=XXX value.  Return the XXX or def.**/
  public static String prop(String[] args, String prop, String def) {
    prop = prop.toUpperCase()+"=";
    for (int i=0; i<args.length; i++) {
      if (args[i].toUpperCase().startsWith(prop)) {
        return args[i].substring(prop.length());
      }
    }
    return def;
  }

  /**Check for a given flag. **/
  public static boolean flag(String[] args, String flag) {
    flag = flag.toUpperCase();
    for (int i=0; i<args.length; i++) {
      if (args[i].toUpperCase().equals(flag)) {return true;}
    }
    return false;
  }

  /**Check for a -key XXX value.  Return the XXX or def.**/
  public static String key(String[] args, String key, String def) {
    key = key.toUpperCase();
    for (int i=0; i<args.length; i++) {
      if (args[i].toUpperCase().equals(key)) {return args[i+1];}
    }
    return def;
  }
}
