#include <raylib.h>
#include <raymath.h>

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <vector>

#include "server.h"

using std::vector;

const int width = 1500;
const int height = 800;

const char *serverIP = "127.0.0.1";
const int serverPort = 12345;

#define RUNSERVER 1

using json = nlohmann::json;

Color ColorLerp(Color a, Color b, float t) {
  t = fmaxf(0, fminf(t, 1)); // Ensure t is in the [0, 1] range
  return (Color){(unsigned char)(a.r + (b.r - a.r) * t),
                 (unsigned char)(a.g + (b.g - a.g) * t),
                 (unsigned char)(a.b + (b.b - a.b) * t),
                 (unsigned char)(a.a + (b.a - a.a) * t)};
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

void drawResults(std::vector<float> res) {
  int size = 30;
  int x = width - 300;
  int y = 100;

  DrawText(("0 1\t" + std::to_string(res[0])).c_str(), x, y, size, WHITE);
  DrawText(("0 1\t" + std::to_string(res[1])).c_str(), x, y + 30, size, WHITE);
  DrawText(("1 0\t" + std::to_string(res[2])).c_str(), x, y + 60, size, WHITE);
  DrawText(("1 1\t" + std::to_string(res[3])).c_str(), x, y + 90, size, WHITE);
}

int main(void) {
  SetTraceLogLevel(LOG_NONE);
  SetConfigFlags(
      FLAG_MSAA_4X_HINT); // Enable 4x MSAA (or FLAG_MSAA_8X_HINT for 8x MSAA)

  InitWindow(width, height, "Raylib template");

  int port = 12345; // Port number

#if RUNSERVER
  Server server(port);
  std::thread serverThread([&server]() { server.start(); });
  serverThread.detach(); // Detach the server thread

  std::thread torchThread([]() {
    if (std::system("python nn.py") == 0) {
      std::cout << "Starting Neural Network" << std::endl;
    } else {
      std::cout << "ERROR: neural network can not statr" << std::endl;
    }
  });
#endif

  std::vector<std::vector<float>> w1, w2;
  std::vector<float> b0, b1, b2, test;

  int layerCount = 3;
  Layer inputLayer(2, layerCount, 0), hiddenLayer(4, layerCount, 1),
      outputLayer(1, layerCount, 2);
  std::vector<Layer> layers = {inputLayer, hiddenLayer, outputLayer};
  while (!WindowShouldClose()) {

#if RUNSERVER
    json receivedData = server.getReceivedData();
    if (!receivedData.empty()) {
      try {
        w1 = (std::vector<vector<float>>)receivedData["layers.0.weight"];
        w2 = (std::vector<vector<float>>)receivedData["layers.2.weight"];
        b0 = (std::vector<float>)receivedData["input.bias"];
        b1 = (std::vector<float>)receivedData["layers.0.bias"];
        b2 = (std::vector<float>)receivedData["layers.2.bias"];

        layers[1].updateWeights(w1);
        layers[2].updateWeights(w2);
        layers[0].updateBias(b0);
        layers[1].updateBias(b1);
        layers[2].updateBias(b2);

        test = (std::vector<float>)receivedData["test"];
      } catch (...) {
      }
    }
#endif

    BeginDrawing();
    ClearBackground(BLACK);

    int j = 0;
    for (int i = 1; i < layerCount; i++) {
      Layer curr = layers[i];
      Layer prev = layers[i - 1];
      for (Node n1 : curr.nodes) {
        for (Node n2 : prev.nodes) {
          Color color = BLUE;
          if (n1.weights[j] <= 0.0)
            color = RED;
          DrawLineEx((Vector2){(float)n1.x, (float)n1.y},
                     (Vector2){(float)n2.x, (float)n2.y},
                     Lerp(1.0, 5.0,
                          (n1.weights[j] > 0 ? n1.weights[j] : -n1.weights[j])),
                     color);
          j++;
        }
        j = 0;
      }
    }

    for (Layer layer : layers) {
      layer.draw();
    }

    if (test.size() > 0)
      drawResults(test);

    EndDrawing();

#ifdef RUNSERVER
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
#endif
  }

#if RUNSERVER
  torchThread.join();
#endif

  CloseWindow();

  return 0;
}
