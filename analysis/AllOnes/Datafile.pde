
class Datafile { 
  HashMap<Integer,ArrayList<Float>> data;
  Experiment expr;
  int end;

  Datafile(String filename, Experiment expr) {
    end = 0;    
    print("loading datafile: " + filename + "...");
    this.expr = expr;
    String[] file = loadStrings(filename);
    data = new HashMap<Integer,ArrayList<Float>>();
    int step = 0;

    for(int i=1; i<file.length; ++i) {
      String l = trim(file[i]);
      if((l.length()==0) || (l.charAt(0)=='#')) {
        continue;
      }
      String[] e = splitTokens(l);
      Integer k = int(e[0]);
      if(!data.containsKey(k)) {
        data.put(k, new ArrayList<Float>());
      }
      data.get(k).add(float(e[2]));
      end = max(end, k); 
    }
    
    println(" done.");
  }
  
  ArrayList<Float> getStep(int e) {
     return data.get(e); 
  }
  
  boolean atEnd(int update) {
    return update >= end;
  }  
}


