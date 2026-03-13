// server.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include <string>
#include <thread>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") 
using namespace std;

//const int PORT = 54321;
const int MAX_NUMBER = 100;
int secretNumber = 0;

// Функция одного клиента (поток)
void handleClient(SOCKET clientSocket, int clientId)
{
    char buffer[32] = { 0 };

    cout << "Клиент" << clientId << " подключился" << endl;

    while (true)
    {
        int receive = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);//получаем данные

        if (receive <= 0) {
            cout << "Клиент" << clientId << " отключился" << endl;
            break;
        }
        string input(buffer);
        try {
            int number = stoi(input);//перевод в целое число

            string message;
            if (number == secretNumber) {
                message = "WIN:" + to_string(secretNumber);
                send(clientSocket, message.c_str(), message.length(), 0);
                cout << "Клиент" << clientId << " Угадал" << endl;
                break;
            }
            else if (number < secretNumber) message = "MORE";
            else message = "LESS";

            receive = send(clientSocket, message.c_str(), message.length(), 0);//отправляем
            if (receive == SOCKET_ERROR) {
                cout << "Ошибка отправки клиенту " << clientId << endl;
                break;
            }
        }
        catch (...)
        {
            string err = "Введите число от 1 до 100\n";
            send(clientSocket, err.c_str(), err.length(), 0);
        }
    }
    closesocket(clientSocket);
}

int main() {
    setlocale(0, "rus");
    // Инициализация Winsock, загрузка библиотеки
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << "WSAStartup failed: " << iResult << endl;
        return 1;
    }

    // Число
    srand(static_cast<unsigned int>
        (time(0)));
    secretNumber = 
        rand() % MAX_NUMBER + 1;
    cout << "Сервер запущен" 
        << endl;
    cout << "Загадано число от 1 до " 
        << MAX_NUMBER << 
        ". Ожидание игроков" << endl;

    //Настройка адреса
    struct addrinfo* result = NULL;
    struct addrinfo* ptr = NULL;
    struct addrinfo hints;

    // ZeroMemory
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;  // IPv4
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;  // Для сервера - bind

    // Определяем порт
#define DEFAULT_PORT "27015"

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << "getaddrinfo error: " << iResult << endl;
        WSACleanup();
        return 1;
    }
    // 3. Создание сокета
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        cout << "Error at socket(): " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // 4. Привязка (Bind)
    if (bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen) == SOCKET_ERROR) {
        cout << "bind failed: " << WSAGetLastError() << endl;
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);
    // 5. Прослушивание (Listen)
    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR) {
        cout << "listen failed: " << WSAGetLastError() << endl;
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    int clientIdCounter = 0;
    SOCKET clientSocket;
    struct sockaddr clientAddr;
    int addrlen = sizeof(clientAddr);

    cout << "Ожидание подключений..." << endl;

    while (true) {
        clientSocket = accept(ListenSocket, (struct sockaddr*)&clientAddr, &addrlen);
        if (clientSocket == INVALID_SOCKET) {
            cout << "accept failed: " << WSAGetLastError() << endl;
            continue;
        }
        clientIdCounter++;
        // Создаём поток для клиента
        thread t(handleClient, clientSocket, clientIdCounter);
        t.detach();
    }

    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}