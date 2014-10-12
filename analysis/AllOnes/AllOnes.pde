String[] filenames = {"/Users/dk/Desktop/AllOnes/population_fitness.dat"};
Experiment expr;
int frame=0;

// 100px / meter
void setup() {
  size(500, 550);
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
//  saveFrame("frames/" + nf(frame++,5) + ".png");
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
  

