#undef UNICODE

#define WIN32_LEAN_AND_MEAN

#include <iostream>
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#include "Server.h"


// фция для обрезки строки
char* slice(char* s, int from, int to)
{
    int j = 0;
    for (int i = from; i <= to; ++i)
        s[j++] = s[i];
    s[j] = 0;
    return s;
}

int startServer(std::string(*func)(std::string), const char* addresses, const char* port = "27015")
{
    // Размер принимаемого буфера
    const int DEFAULT_BUFLEN = 512;

    WSADATA wsaData;
    int iResult;

    SOCKET ListenSocket = INVALID_SOCKET;
    SOCKET ClientSocket = INVALID_SOCKET;

    struct addrinfo* result = NULL;
    struct addrinfo hints;

    int iSendResult;
    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    // Инициализация Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    // адрес сервера и порт
    iResult = getaddrinfo(addresses, port, &hints, &result);
    if (iResult != 0) {
        printf("getaddrinfo failed with error: %d\n", iResult);
        WSACleanup();
        return 1;
    }

    // Создание сокета для подключения к серверу
    ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    // Установка сокета для прослушивания TCP
    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(result);

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }


    // Цикл для прослушки
    while (true)
    {
        // Принять клиентский сокет
        ClientSocket = accept(ListenSocket, NULL, NULL);
        if (ClientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(ListenSocket);
            WSACleanup();
            return 1;
        }

        // No longer need server socket
        // closesocket(ListenSocket);

        std::string strResult = "";
        // Принимать до тех пор, пока одноранговый узел не отключит соединение
        do {

            iResult = recv(ClientSocket, recvbuf, recvbuflen, 0);
            if (iResult > 0) {
                strResult += slice(recvbuf, 0, iResult - 1);
            }
            else if (iResult == 0)
                printf("\nConnection closing...\n");
            else {
                printf("recv failed with error: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }

        } while (iResult > 0);

        std::cout << "Сообщение от клиента: " << strResult << std::endl;


        // Проверка на стоп слова
        if (strResult == "/stopserver")
        {
            std::cout << std::endl << "Остановка сервера..." << std::endl;
            break;
        }

        // Обработка сообщения:
        std::string str = func(strResult);
        char* cstr = &str[0];

        // Отправить результат обратно отправителю
        iSendResult = send(ClientSocket, cstr, (int)strlen(cstr), 0);
        if (iSendResult == SOCKET_ERROR)
        {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

        // отключите соединение, так как мы закончили
        iResult = shutdown(ClientSocket, SD_SEND);
        if (iResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }

    }
    // cleanup
    closesocket(ClientSocket);
    WSACleanup();
    return 0;
}