#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <algorithm>
#include <map>
#include <set>

#include <GL/gl.h>
#include <GL/glut.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

struct Block {
  char x_min, y_min, x_max, y_max;
  Block() : x_min(0), y_min(0), x_max(1), y_max(1) {};
  Block(char x_min_, char y_min_, char x_max_, char y_max_) :
    x_min(x_min_), y_min(y_min_), x_max(x_max_), y_max(y_max_) {};
  Block translate(char x, char y) const {
    return Block(x_min + x, y_min + y, x_max + x, y_max + y);
  }
  bool operator<(const Block b) const {
    return std::array<char,4>({x_min, y_min, x_max, y_max})
      < std::array<char,4>({b.x_min, b.y_min, b.x_max, b.y_max});
  }
  bool operator==(const Block b) const {
    return std::array<char,4>({x_min, y_min, x_max, y_max})
      == std::array<char,4>({b.x_min, b.y_min, b.x_max, b.y_max});
  }
  bool operator&(const Block b) const {
    if (x_max < b.x_min) return false;
    if (x_min > b.x_max) return false;
    if (y_max < b.y_min) return false;
    if (y_min > b.y_max) return false;
    return true;
  }
  bool in(const Block b) const {
    return (x_min >= b.x_min &&
            y_min >= b.y_min &&
            x_max <= b.x_max &&
            y_max <= b.y_max);
  }
  void draw(void) const {
    glBegin(GL_QUADS);
    glVertex2d(x_min,y_min);
    glVertex2d(x_max,y_min);
    glVertex2d(x_max,y_max);
    glVertex2d(x_min,y_max);
    glEnd();
  }
};

const Block area = Block(-2,-2, 40, 50);

typedef std::array<Block,10> World;

void draw(const World & w) {
  glClear(GL_COLOR_BUFFER_BIT);
  for (auto b: w) b.draw();
}

bool check(World w) {
  for (auto b: w)
    if (!b.in(area))
      return false;
  for (int i = 0; i < w.size(); i++)
    for (int j = i+1; j < w.size(); j++)
      if (w[i] & w[j])
        return false;
  return true;
}

bool reach(World w) {
  Block & m = w[9];
  return m.x_min == 10 && m.y_min == 0;
}

std::vector<World> next(World w) {
  std::vector<World> n;
  for (auto & b: w) {
    b=b.translate(10, 0);
    if (check(w)) {
      n.push_back(w);
      b=b.translate(10, 0);
      if (check(w)) {
        n.push_back(w);
      }
      b=b.translate(-10, 0);
    }
    b=b.translate(-10, 0);

    b=b.translate(-10, 0);
    if (check(w)) {
      n.push_back(w);
      b=b.translate(-10, 0);
      if (check(w)) {
        n.push_back(w);
      }
      b=b.translate(10, 0);
    }
    b=b.translate(10, 0);

    b=b.translate(0, 10);
    if (check(w)) {
      n.push_back(w);
      b=b.translate(0, 10);
      if (check(w)) {
        n.push_back(w);
      }
      b=b.translate(0, -10);
    }
    b=b.translate(0, -10);

    b=b.translate(0, -10);
    if (check(w)) {
      n.push_back(w);
      b=b.translate(0, -10);
      if (check(w)) {
        n.push_back(w);
      }
      b=b.translate(0, 10);
    }
    b=b.translate(0, 10);

  }
  return std::move(n);
}


World initWorld = {
  Block(0, 0, 8, 8).translate(0,0),
  Block(0, 0, 8, 8).translate(10,10),
  Block(0, 0, 8, 8).translate(20,10),
  Block(0, 0, 8, 8).translate(30,0),

  Block(0, 0, 8, 18).translate(0,10),
  Block(0, 0, 8, 18).translate(0,30),
  Block(0, 0, 8, 18).translate(30,10),
  Block(0, 0, 8, 18).translate(30,30),

  Block(0, 0, 18, 8).translate(10,20),
  Block(0, 0, 18, 18).translate(10,30)
};

int main(int argc, char ** argv) {
  const int width = 21*8;
  const int height = 26*8;
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_SINGLE);
  glutInitWindowSize(width, height);
  glutCreateWindow("huarongdao");
  glClearColor(1, 1, 1, 1);
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3d(0,0,0);
  glOrtho(area.x_min, area.x_max, area.y_min, area.y_max, -1, 1);

  std::map<World, std::set<World>> trace;
  trace[initWorld] = std::set<World>();
  std::vector<World> old = {initWorld};
  std::set<World> target;

  draw(initWorld);
  glFlush();

  // BFS
  while (!old.empty()) {
    std::vector<World> n;
    for (auto w : old)
      for (auto & nw : next(w)) {
        std::sort(nw.begin(), nw.begin() + 4);
        std::sort(nw.begin()+4, nw.begin() + 8);
        if (std::find(n.begin(), n.end() ,nw) != n.end()) {
          std::set<World> t(trace[nw]);
          t.insert(w);
          trace[nw] = t;
        } else if (trace.find(nw) == trace.end()) {
          trace[nw] = std::set<World>({w});
          if (reach(nw)) target.insert(nw);
          else n.push_back(nw);
        }
      }
    if (n.empty()) break;
    old = std::move(n);
  }

  std::cerr << target.size() << std::endl;

  std::set<World> all_worlds(target);
  // trace forth from trace back
  std::map<World, std::set<World>> trace_f;
  while (!target.empty()) {
    std::set<World> ntarget;
    for (auto t : target)
      for (auto s : trace[t]) {
        if (trace_f.find(s) == trace_f.end()) {
          trace_f[s] = std::set<World>({t});
          ntarget.insert(s);
        } else {
          std::set<World> ts = trace_f[s]; ts.insert(t);
          trace_f[s] = ts;
        }
      }
    target = std::move(ntarget);
    all_worlds.insert(target.begin(), target.end());
  }

  std::cerr << trace_f.size() << std::endl;

  std::vector<World> worlds(all_worlds.begin(), all_worlds.end());
  std::map<World, size_t> wid;
  for (size_t i = 0; i < worlds.size(); i++) {
    World w = worlds[i];
    wid[w] = i;
    draw(w); glFlush();
    {
      char fname[256] = "";
      std::array<std::array<std::array<unsigned char,4>,width>,height> buf;
      glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, &buf);
      std::reverse(buf.begin(), buf.end());
      sprintf(fname, "%d.png", i);
      stbi_write_png(fname, width, height, 4, &buf, 4 * width * sizeof(unsigned char));
    }
  }

  {
    std::ofstream ofs("graph.dot");
    ofs << "digraph {" << std::endl;
    ofs << "node [shape=box]" << std::endl;
    for (size_t i = 0; i < worlds.size(); i++)
      ofs << i << "[image=\""<<i<<".png\" label=\"\"]" << std::endl;
    for (auto wp : trace_f) {
      for (auto n: wp.second) {
        ofs << wid[wp.first] << " -> " << wid[n]
            << std::endl;
      }
    }
    ofs << "}" << std::endl;
  }
  system("dot -O -Tpng graph.dot");

  return 0;
}
