class Experiment {
  int currentUpdate;
  boolean paused;

  Datafile df;
  String[] filenames;
  int currentFile;

  Experiment() {    
    currentUpdate = 0;
    currentFile = 0;
  }
  
  void setDatafiles(String[] f) {
    filenames = f;
    df = new Datafile(filenames[0], this);
  }
  
  int getCurrentUpdate () {
    return currentUpdate; 
  }
  
  int getCurrentFile() { 
    return currentFile;
  }
  
  void reset() {
    currentUpdate=0;
    paused = true;
    redraw();
  }

  void step(int direction) {
    if(atEnd() && ((currentFile+1) < filenames.length)) {
      currentFile++;
      df = new Datafile(filenames[currentFile], this);
      reset();
      paused = false;
    }
    
    if((direction > 0) && atEnd()) {
      pause();
      return;
    }
    if((direction < 0) && atBegin()) {
      pause();
      return;
    }
    currentUpdate += direction;
    redraw();
  }
  
  void togglePause() {
    if(paused) {
      unpause();
    } 
    else {
      pause();
    }
  }

  void pause() {
    paused = true;
    noLoop();
  }

  void unpause() {
    paused = false;
    loop();
  }
  
  boolean atBegin() {
    return (currentUpdate <= 0);
  }

  boolean atEnd() {    
    return (df.atEnd(currentUpdate));
  }
  
 void draw() {
    String[] line = df.getStep(currentUpdate);
    strokeWeight(4);
    background(255);
    fill(0);
    text("generation: " + str(df.getGeneration()), 10, 10, 100, 20);
    pushMatrix();
    int x = 240 + int(100*float(line[1]));
    translate(x, 90);
    rect(0, 0, 20, 6);
    x = 3;
    int y = 10;
    float m = tan(float(line[2]));
    int px = int(x + 50/sqrt(1+m*m));
    int py = int(y + m * 50/sqrt(1+m*m));
    rotate(-PI/2);
    line(x, y, px, py);
    popMatrix();
  }
}

