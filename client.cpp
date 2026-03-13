// clientKris.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <cstring>
using namespace std;

#pragma comment(lib, "Ws2_32.lib")
#define GREEN "\033[32m"
#define RED   "\033[31m"
#define YELLOW "\033[33m"
#define RESET "\033[0m"

const char* DEFAULT_SERVER = "192.168.1.6"; // По умолчанию localhost
const char* DEFAULT_PORT = "27015";

int main(int argc, char* argv[]) {
    setlocale(0, "rus");

    // Определение IP сервера
    const char* serverIp = (argc > 1) ? argv[1] : DEFAULT_SERVER;
    WSADATA wsaData;
    int iResult;

    //Инициализация Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        cout << RED << "WSAStartup failed: " << iResult << RESET << endl;
        return 1;
    }

    // Настройка адреса через getaddrinfo
    struct addrinfo* result = NULL;
    struct addrinfo hints;

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_UNSPEC;      // IPv4 или IPv6
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_protocol = IPPROTO_TCP;

    iResult = getaddrinfo(serverIp, DEFAULT_PORT, &hints, &result);
    if (iResult != 0) {
        cout << RED << "getaddrinfo error: " << iResult << RESET << endl;
        WSACleanup();
        return 1;
    }

    //Создание сокета
    SOCKET ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET) {
        cout << RED << "Error at socket(): " << WSAGetLastError() << RESET << endl;
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    //Подключение
    iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        cout << RED << "Не удалось подключиться к серверу: " << WSAGetLastError() << RESET << endl;
        closesocket(ConnectSocket);
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    freeaddrinfo(result);
    cout << GREEN << "Введите число от 1 до 100:" << RESET << endl;

    const int buflen = 512;
    char recvbuf[buflen];
    char sendbuf[256];
    bool game_over = false;

    while (!game_over)
    {
        // Сброс буфера перед вводом
        memset(sendbuf, 0, sizeof(sendbuf));
        if (!(cin >> sendbuf))
        {
            break;
        }
        // Отправка данных серверу
        iResult = send(ConnectSocket, sendbuf, (int)strlen(sendbuf), 0);
        if (iResult == SOCKET_ERROR) {
            cout << RED << "Ошибка отправки" << WSAGetLastError() << RESET << endl;
            break;
        }

        //Прием ответа (Блокирующий recv)
        iResult = recv(ConnectSocket, recvbuf, buflen - 1, 0);

        if (iResult > 0) {
            recvbuf[iResult] = '\0';
            string response(recvbuf);


            if (response.find("WIN") != string::npos) {
                cout << GREEN << "\nОтгадали число" << RESET << endl;
                // Извлекаем число после "WIN:"
                size_t pos = response.find(':');
                if (pos != string::npos) {
                    cout << GREEN << "Загаданное число" << response.substr(pos + 1) << RESET << endl;
                }
                game_over = true;
            }
            else if (response == "MORE") {
                cout << YELLOW << "Нужно больше" << RESET << endl;
            }
            else if (response == "LESS") {
                cout << YELLOW << "Нужно меньше" << RESET << endl;
            }
            else if (response.find("ERROR") != string::npos) {
                cout << RED << "Ошибка" << response.substr(6) << RESET << endl;
            }
            else
            {
                cout << "Получено" << response << endl;
            }
        }
        else if (iResult == 0)
        {
            cout << "\nСоединение закрыто сервером" << endl;
            game_over = true;
        }
        else
        {
            cout << RED << "Ошибка приема" << WSAGetLastError() << RESET << endl;
            game_over = true;
        }
    }
    //Завершение соединения (Shutdown)
    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        cout << "Ошибка shutdown: " << WSAGetLastError() << endl;
    }
    closesocket(ConnectSocket);
    WSACleanup();

    cout << GREEN << "Игра окончена" << RESET << endl;
    return 0;
}