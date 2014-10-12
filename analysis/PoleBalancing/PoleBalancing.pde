String[] filenames = {
  "/Users/dk/Desktop/PoleBalancing/pole_movie_0.dat",
  "/Users/dk/Desktop/PoleBalancing/pole_movie_100.dat",
  "/Users/dk/Desktop/PoleBalancing/pole_movie_200.dat",
  "/Users/dk/Desktop/PoleBalancing/pole_movie_300.dat",
  "/Users/dk/Desktop/PoleBalancing/pole_movie_500.dat",
  "/Users/dk/Desktop/PoleBalancing/pole_movie_1000.dat"
};

Experiment expr;
int frame=0;

// 100px / meter
void setup() {
  size(480, 100);
  frameRate(30);
  expr = new Experiment();
  expr.setDatafiles(filenames);  
}

void draw() {
  clear();
  if (!expr.paused) {
    expr.step(1);
  }
  expr.draw();
  saveFrame("frames/" + nf(frame++,5) + ".png");
} 

void keyPressed() {
  switch(key) {
 
  case 'P':
  case 'p': 
    {
      expr.togglePause();
      break;
    }
      case 'Q':
  case 'q': 
    {
      exit();
    }
  case 'r': 
    {
      expr.reset();
      break;
    }
  case 'S': 
    {
      expr.step(-1);
      break;
    }
  case 's': 
    {
      expr.step(1);
      break;
    }
  }
}
  

