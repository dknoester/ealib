import java.util.Collections;



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
  
  color getColor(float value) {
    float[][] C = { {0,0,1}, {0,1,0}, {1,1,0}, {1,0,0} };
    // A static array of 4 colors:  (blue,   green,  yellow,  red) using {r,g,b} for each.
   
    // expected value == .5
    value -= 0.5;
    value = max(0.0, value); // clip at 0
    value *= 2;
 
    int idx1;        // |-- Our desired color will be between these two indexes in "color".
    int idx2;        // |
    float fractBetween = 0;  // Fraction between "idx1" and "idx2" where our value is.
 
    value = value * 3;        // Will multiply value by 3.
    idx1  = floor(value);                  // Our desired color will be after this index.
    idx2  = idx1+1;                        // ... and before this index (inclusive).
    fractBetween = value - float(idx1);    // Distance between the two indexes (0-1).

    int r = int(255*((C[idx2][0] - C[idx1][0])*fractBetween + C[idx1][0]));
    int g = int(255*((C[idx2][1] - C[idx1][1])*fractBetween + C[idx1][1]));
    int b = int(255*((C[idx2][2] - C[idx1][2])*fractBetween + C[idx1][2]));
    return color(r,g,b); 
  }
  
  
  
 void draw() {
    ArrayList<Float> A = df.getStep(currentUpdate);
    Collections.shuffle(A);
//    strokeWeight(4);
    background(255);
    fill(0);
    text("generation: " + str(currentUpdate/10), 10, 10, 100, 20);
    fill(255);   
    for(int i=0; i<100; ++i) {
      int x = 50*(i%10);
      int y = 50*(i/10);
      pushMatrix();
      translate(x, y+50);
      fill(getColor(A.get(i)));
      rect(0, 0, 50, 50);
      popMatrix();
    }
//    int x = 240 + int(100*float(line[1]));
//    translate(x, 90);
//    rect(0, 0, 20, 6);
//    x = 3;
//    int y = 10;
//    float m = tan(float(line[2]));
//    int px = int(x + 50/sqrt(1+m*m));
//    int py = int(y + m * 50/sqrt(1+m*m));
//    rotate(-PI/2);
//    line(x, y, px, py);
//    popMatrix();
  }
}

