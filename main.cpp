#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <thread>
#include <mutex>
#include <vector>

using namespace std;


class Client {
private:
    int clientSocket;
    sockaddr_in serverAddr{};
    thread sendThread, receiveThread;

    void createSocket(const char* serverIp, int port) {
        clientSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (clientSocket == -1) {
            perror("Socket creation failed");
            exit(1);
        }

        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(port);
        inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);
    }

    void connectToServer() {
        if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
            perror("Connection failed");
            exit(1);
        }
    }

    void sendMessage() const {
        cout << "TYPE START TO START and MOVE X Y TO MOVE " << endl;
        while (true){
            string message;
            getline(cin, message);
            if (message == "START" || message == "QUIT") {
                send(clientSocket, message.c_str(), message.length(), 0);
            } else if (message.length() >= 8 && message.find("MOVE") == 0 && isdigit(message[5]) && isdigit(message[7])) {
                send(clientSocket, message.c_str(), message.length(), 0);
            } else {
                cout << "Invalid command" << endl;
            }
        }
    }

    void receiveMessage() {
        char buffer[1024];

        while (true){
            memset(buffer, 0, sizeof(buffer));
            int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesReceived <= 0) {
                close(clientSocket);
                if (bytesReceived == 0) {
                    cout << "Server closed the connection" << endl;
                } else {
                    perror("Failed to receive message");
                }
                break;
            }
            string bufferStr = string(buffer, bytesReceived);
            handleCommand(bufferStr);
        }

    }

    void handleCommand(string command){
        if (command == "QUIT"){
            cout << "Successfully disconnected" << endl;
            close(clientSocket);
            exit(0);
        } else{
            cout << command << endl;
        }
    }

public:
    Client(const char* serverIp, int port) {
        createSocket(serverIp, port);
        connectToServer();
    }

    ~Client() {
        if (sendThread.joinable()) sendThread.join();
        if (receiveThread.joinable()) receiveThread.join();
        close(clientSocket);
    }

    void start() {
        sendThread = thread(&Client::sendMessage, this);
        receiveThread = thread(&Client::receiveMessage, this);

        sendThread.join();
        receiveThread.join();
    }
};

int main() {
    int port = 12345;
    const char *serverIp = "127.0.0.1";

    Client client(serverIp, port);

    client.start();

    return 0;
}