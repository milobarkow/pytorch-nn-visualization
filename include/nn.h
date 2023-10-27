#include <raylib.h>

#include <cstdio>
#include <string>
#include <vector>

Color ColorLerp(Color a, Color b, float t) {
  t = fmaxf(0, fminf(t, 1)); // Ensure t is in the [0, 1] range
  return (Color){(unsigned char)(a.r + (b.r - a.r) * t),
                 (unsigned char)(a.g + (b.g - a.g) * t),
                 (unsigned char)(a.b + (b.b - a.b) * t),
                 (unsigned char)(a.a + (b.a - a.a) * t)};
}

void drawResults(std::vector<float> res) {
  int width = GetScreenWidth();
  int height = GetScreenHeight();
  int size = 30, x = width - 300, y = 100;

  DrawText(("0 1\t" + std::to_string(res[0])).c_str(), x, y, size, WHITE);
  DrawText(("0 1\t" + std::to_string(res[1])).c_str(), x, y + 30, size, WHITE);
  DrawText(("1 0\t" + std::to_string(res[2])).c_str(), x, y + 60, size, WHITE);
  DrawText(("1 1\t" + std::to_string(res[3])).c_str(), x, y + 90, size, WHITE);
}

struct Node {
  int x, y, rad, weightCount = 0;
  std::vector<float> weights;
  float val = -1;
  Node(int x, int y, int rad, int wc = 0)
      : x(x), y(y), rad(rad), weightCount(wc) {
    weights.resize(2);
  }

  void draw() {
    Color color = ColorLerp(RED, BLUE, val);
    DrawCircle(x, y, rad, color);
  }

  void drawBias() {
    char cStrVal[8];
    snprintf(cStrVal, sizeof(cStrVal), "%.6f", val);
    const char *actualVal = cStrVal;
    DrawText(cStrVal, x - 30, y - 10, 20, BLACK);
  }
};

struct Layer {
  const int nodeSize = 45;
  int ID;
  std::vector<Node> nodes;

  Layer(int size, int layerCount, int layerID) : ID(layerID) {
    int width = GetScreenWidth();
    int height = GetScreenHeight();
    int x, y, intervalSize = 0, y_Padding = 100, x_Padding = 200;
    x = (x_Padding + layerID * ((width - x_Padding * 2) / (layerCount - 1)));
    if (size == 1) {
      nodes.push_back(Node(x, height / 2, nodeSize));
      return;
    }
    intervalSize = (height - y_Padding * 2) / (size - 1);
    for (int i = 0; i < size; i++) {
      y = y_Padding + i * intervalSize;
      nodes.push_back(Node(x, y, nodeSize));
    }
  }

  void updateBias(std::vector<float> bias) {
    for (int i = 0; i < bias.size(); i++) {
      nodes[i].val = bias[i];
    }
  }
  void updateWeights(std::vector<std::vector<float>> weights) {
    for (int i = 0; i < weights.size(); i++) {
      nodes[i].weights = weights[i];
    }
  }

  void draw() {
    for (Node node : nodes) {
      node.draw();
      node.drawBias();
    }
  }
};
