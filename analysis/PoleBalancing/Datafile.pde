
class Datafile { 
  HashMap<Integer,String[]> data;
  Experiment expr;
  int end;
  int generation;

  Datafile(String filename, Experiment expr) {
    String[] m = match(filename, "(\\d+)\\.dat");
    generation = int(m[1]) / 10;
    
    print("loading datafile: " + filename + "...");
    this.expr = expr;
    String[] file = loadStrings(filename);
    data = new HashMap<Integer,String[]>();
    int step = 0;

    for(int i=1; i<file.length; ++i) {
      String l = trim(file[i]);
      if((l.length()==0) || (l.charAt(0)=='#')) {
        continue;
      }
      String[] e = splitTokens(l);
      data.put(step, e);
      ++step;
    }
    
    end = --step;
    println(" done.");
  }
  
  int getGeneration() {
     return generation;
  }

  String[] getStep(int e) {
     return data.get(e); 
  }
  
  boolean atEnd(int update) {
    return update >= end;
  }  
}


