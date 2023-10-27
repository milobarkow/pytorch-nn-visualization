import torch
import torch.nn as nn
import torch.optim as optim
import json
import socket


class NN(nn.Module):
    def __init__(self, input_size, hidden_size, output_size):
        super(NN, self).__init__()  # Call the constructor of the parent class

        self.layers = nn.Sequential(
            nn.Linear(input_size, hidden_size, bias=True),
            nn.ReLU(),
            nn.Linear(hidden_size, output_size),
            nn.Sigmoid()
        )

    def forward(self, x):
        return self.layers(x)

    def export_data(self, data):
        HOST = '127.0.0.1'  # IP address of the server
        PORT = 12345  # Port number

        json_data = json.dumps(data)

        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as client_socket:
            client_socket.connect((HOST, PORT))
            client_socket.sendall(json_data.encode())

    def train(self, input_data, target_data):
        criterion = nn.BCELoss()
        optimizer = optim.SGD(self.parameters(), lr=0.01)
        data = {}
        epoch = 0
        while True:
            outputs = self(input_data)
            loss = criterion(outputs, target_data)
            optimizer.zero_grad()
            loss.backward()
            optimizer.step()

            if (epoch + 1) % 1000 == 0:
                print(f'Epoch [{epoch + 1}/10000], Loss: {loss.item():.4f}')
            for name, param in model.named_parameters():
                if param.requires_grad:
                    data[name] = param.data.tolist()
            data["input.bias"] = [0.0, 1.0]
            data["test"] = self.test(input_data)
            self.export_data(data)
            epoch += 1

    def test(self, test_data):
        with torch.no_grad():
            predictions = self(test_data)
        return [x[0] for x in predictions.tolist()]


if __name__ == '__main__':
    input_size = 2
    hidden_size = 4
    output_size = 1

    model = NN(input_size, hidden_size, output_size)

    x = torch.tensor([[0, 0], [0, 1], [1, 0], [1, 1]],
                     dtype=torch.float32)  # Input data
    y = torch.tensor([[0], [1], [1], [0]], dtype=torch.float32)  # Target data

    model.train(x, y)

    model.test(x)
