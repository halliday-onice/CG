#include <GLUT/glut.h>
#include <iostream>
#include <vector>
#include <cmath>


/* MAIN IDEIA

Group every 4 points into a separate square.

Store them in a 2D vector (std::vector<std::vector<std::pair<int, int>>>), where each subvector contains 4 points (a square).

Start a new group after 4 points.

In display(), draw each group as a GL_LINE_LOOP or filled GL_POLYGON.

*/

// Vector to hold groups of 4 points (squares)
std::vector<std::vector<std::pair<int, int>>> squares;

// Temporary vector to accumulate 4 points
std::vector<std::pair<int, int>> currentSquare;

int windowWith = 500;
int windowHeight = 500;

std::vector<std::pair<int, int>> sortPointsCounterClockwise(const std::vector<std::pair<int, int>>& points) {
      // Calculate centroid
      float centerX = 0.0f, centerY = 0.0f;
      for (const auto& p : points) {
          centerX += p.first;
          centerY += p.second;
      }
      centerX /= points.size();
      centerY /= points.size();
  
      // Sort by angle from center
      std::vector<std::pair<int, int>> sorted = points;
      std::sort(sorted.begin(), sorted.end(), [=](auto& a, auto& b) {
          float angleA = atan2(a.second - centerY, a.first - centerX);
          float angleB = atan2(b.second - centerY, b.first - centerX);
          return angleA < angleB;
      });
  
      return sorted;
}

void display(){
      glClear(GL_COLOR_BUFFER_BIT);
      glPointSize(5.0f);
      glColor3f(1.0f, 0.0f, 0.0f);  // red dots for visibility
  
      glBegin(GL_POINTS);
      for(const auto& square: squares){
            for (const auto& pos : square) {
                  float x = static_cast<float>(pos.first);
                  float y = static_cast<float>(windowHeight - pos.second);
                  glVertex2f(x, y);
            }
      }

      for(const auto& pos: currentSquare){
            float x = static_cast<float>(pos.first);
            float y = static_cast<float>(windowHeight - pos.second);
            glVertex2f(x, y);
      }
      
      glEnd();

      //now I draw the squares here
      glColor3f(0.0f, 0.0f, 1.0f); //blue squares
      for(const auto& square: squares){
            if(square.size() == 4){
                  glBegin(GL_LINE_LOOP);
                  for(const auto& pos: square){
                        float x = static_cast<float>(pos.first);
                        float y = static_cast<float>(windowHeight - pos.second);
                        glVertex2f(x, y);
                  }
                  glEnd();
    
            }
      }
  
      glFlush();
}
void handleKeyboard(unsigned char key, int x, int y){
      if (key == 27) { // 27 = ASCII code for ESC
            std::cout << "ESC key pressed. Exiting.\n";
            exit(0);
      }
      
      

}

void mouse(int button, int state, int x, int y){
      
      if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN){
            std::cout << "Left button clicked (" << x << ", " << y << ")\n";
            currentSquare.push_back({x,y});
            std::cout << "Point: (" << x << ", " << y << ")\n";
            
            if(currentSquare.size() == 4){
                  auto sorted = sortPointsCounterClockwise(currentSquare);
                  squares.push_back(sorted);
                  currentSquare.clear();
            }
            glutPostRedisplay();
      } 
}

void reshape(int w, int h) {
      windowWith = w;
      windowHeight = h;
      glViewport(0, 0, w, h);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      gluOrtho2D(0, w, 0, h);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
}

int main(int argc, char **argv){
      glutInit(&argc, argv);
      glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
      glutInitWindowSize(windowWith, windowHeight);
      glutCreateWindow("Draw Dots on Click");

      glClearColor(1, 1, 1, 1);


      glutDisplayFunc(display);
      glutMouseFunc(mouse);
      glutKeyboardFunc(handleKeyboard);
      glutReshapeFunc(reshape);
      
      

      glutMainLoop();
      return 0;
}