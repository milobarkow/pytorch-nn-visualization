#include <raylib.h>
#include <raymath.h>

#include <vector>

#include "nn.h"
#include "server.h"

const int width = 1500;
const int height = 800;

const int port = 12345;

#define RUNSERVER 1

using std::vector;
using json = nlohmann::json;

int main(void) {
  SetTraceLogLevel(LOG_NONE);
  SetConfigFlags(FLAG_MSAA_4X_HINT);

  InitWindow(width, height, "Raylib template");

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
