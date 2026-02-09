#include "network.h"
#include <iostream>
#include <string>
#include <thread>

#define _USE_MATH_DEFINES
#include <fmod.hpp>
#include <fmod_errors.h>
#include <cmath>
#include <thread>
#include <chrono>
#include <conio.h>
#include <vector>
#pragma comment(lib, "fmod_vc.lib")
#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFFER_SIZE 1024

class Sound {
public:
    static Sound& getInstance() {
        static Sound instance;
        return instance;
    }

    void play(float volume = 1.f) {
        system->playSound(sound, nullptr, false, &channel);
        if (channel) channel->setVolume(volume);
    }

    void playDM(float volume = 1.f) {
        system->playSound(soundDM, nullptr, false, &channel);
        if (channel) channel->setVolume(volume);
    }
    void update() {
        system->update();
    }

private:
    FMOD::System* system = NULL;
    FMOD::Sound* sound = NULL;
    FMOD::Sound* soundDM = NULL;
    FMOD::Channel* channel = NULL;
    Sound() {
        FMOD::System_Create(&system);
        system->init(512, FMOD_INIT_NORMAL, NULL);
        system->createSound("msg.mp3", FMOD_DEFAULT, NULL, &sound);
        system->createSound("private.mp3", FMOD_DEFAULT, NULL, &soundDM);

    }
    ~Sound() {
        system->close();
        system->release();
    }
};

std::string MsgToString(const Message& msg) {
    return msg.sender + "|" + msg.receiver + "|" + msg.content;
}

Message StringToMsg(const std::string& str) {
    size_t pos1 = str.find('|');
    size_t pos2 = str.find('|', pos1 + 1);

    std::string sender = str.substr(0, pos1);
    std::string receiver = str.substr(pos1 + 1, pos2 - pos1 - 1);
    std::string content = str.substr(pos2 + 1);

    return Message(content, sender, receiver);
}

Client::Client() {}
Client::~Client() { disconnect(); }

void Client::recvThread(SOCKET sock) {
    char buffer[DEFAULT_BUFFER_SIZE];

    while (true) {
        int len = recv(sock, buffer, DEFAULT_BUFFER_SIZE - 1, 0);
        if (len <= 0) {
            std::cout << "Disconnected from server.\n";
            break;
        }

        buffer[len] = '\0';
        Message tmp = StringToMsg(std::string(buffer));
        if (tmp.sender != "System")
        {
            if (tmp.receiver != "all")
            {
                std::cout << "Received private message from " << tmp.sender << ": " << tmp.content << std::endl;
                Sound::getInstance().playDM();
            }
            else
            {
                Sound::getInstance().play();
            }
			//newmsg = true;
            std::lock_guard<std::mutex> lock(msgMutex);
            msgs.push_back(tmp);
			//user changed, update user list
            if (tmp.sender=="Notification")
            {
				newuser = true;
            }
        }
		else//system message dont show
        {
			userList = tmp.content;
            std::cout <<"server sent:"<< userList << std::endl;
        }
        //std::cout << buffer << std::endl;
    }
    return;
}

std::string Client::updateUserList() {
	std::cout << "network: " << userList << std::endl;
	newuser = false;
    return userList;
}

void Client::connect_to_server(const std::string& _username) {
    const char* host = "127.0.0.1"; // Server IP address
    unsigned int port = 65432;

    // Initialize WinSock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return;
    }

    // Create a socket
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    // Resolve the server address and port
    sockaddr_in server_address = {};
    server_address.sin_family = AF_INET;
    //server_address.sin_port = htons(std::stoi(port));
    server_address.sin_port = htons(port);
    if (inet_pton(AF_INET, host, &server_address.sin_addr) <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return;
    }

    // Connect to the server
    if (connect(client_socket, reinterpret_cast<sockaddr*>(&server_address), sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "Connection failed with error: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return;
    }

    std::cout << "Connected to the server." << std::endl;
    //std::cout << "Please enter your name:" << std::endl;
    send(client_socket, _username.c_str(), (int)_username.size(), 0);
	username = _username;
    // Start a thread to receive messages from the server
    recv_thread = std::thread(&Client::recvThread, this, client_socket);
    recv_thread.detach();
    return;
}

void Client::sendMessage(const std::string& content, std::string receiver) {
	Message msg(content, username, receiver);
	std::string msg_str = MsgToString(msg);
    if (send(client_socket, msg_str.c_str(), (int)msg_str.size(), 0) == SOCKET_ERROR) {
        std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return;
    }
}

std::vector<Message> Client::getMessage() 
{
    std::vector<Message> public_message;
    public_message.clear();
    std::lock_guard<std::mutex> lock(msgMutex);
    for (auto& m : msgs)
    {
        //std::cout << m.sender << "|" << m.receiver << "|" << m.content << std::endl;
        if (m.receiver == "all")
        {
            //std::cout << m.content << std::endl;
            public_message.push_back(m);
        }
    }
    //newmsg = false;
    return public_message;
}

std::vector<Message> Client::getPrivateMessage(const std::string& user1, const std::string& user2)
{
    std::vector<Message> private_message;
	private_message.clear();
    std::lock_guard<std::mutex> lock(msgMutex);
    for (auto& m : msgs)
    {
        /*if (m.receiver != "all")
        {
            std::cout << "m:" << m.sender << "|" << m.receiver << "|" << m.content << std::endl;
            std::cout << "user:" << user1 << "|" << user2 << "|" << m.content << std::endl;
        }*/
        if ((m.sender == user1 && m.receiver == user2) || (m.sender == user2 && m.receiver == user1))
        {
            //std::cout << m.content << std::endl;
            std::cout << "m:" << m.sender << "|" << m.receiver << "|" << m.content << std::endl;
            private_message.push_back(m);
        }
    }
    //newmsg = false;
	return private_message;
}

void Client::disconnect()
{
    std::string tmp = "byebye!";
    sendMessage(tmp, "!bye");
    closesocket(client_socket);
    //if (recvThread.joinable()) recvThread.join();
    WSACleanup();
}

//void run_client_loop() {
//    std::string sentence = "Hello, server!";
//
//    while (true) {
//        //std::cin >> sentence; 
//        //std::cout << "Send to server: ";
//        std::getline(std::cin, sentence);
//
//        // Send the sentence to the server
//        if (send(client_socket, sentence.c_str(), static_cast<int>(sentence.size()), 0) == SOCKET_ERROR) {
//            std::cerr << "Send failed with error: " << WSAGetLastError() << std::endl;
//            closesocket(client_socket);
//            WSACleanup();
//            return;
//        }
//
//        //std::cout << "Sent: " << sentence << std::endl;
//
//        // Receive the reversed sentence from the server
//        //char buffer[DEFAULT_BUFFER_SIZE] = { 0 };
//        //int bytes_received = recv(client_socket, buffer, DEFAULT_BUFFER_SIZE - 1, 0);
//        //if (bytes_received > 0) {
//        //    buffer[bytes_received] = '\0'; // Null-terminate the received data
//        //    std::cout << "Received from server: " << buffer << std::endl;
//        //}
//        //else if (bytes_received == 0) {
//        //    std::cout << "Connection closed by server." << std::endl;
//        //}
//        //else {
//        //    std::cerr << "Receive failed with error: " << WSAGetLastError() << std::endl;
//        //}
//
//        if (sentence == "!bye") break;
//    }
//    std::cout << "Closing connection\n";
//
//    // Cleanup
//    closesocket(client_socket);
//    WSACleanup();
//}