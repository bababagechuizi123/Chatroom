#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <mutex>
#include <unordered_map>

#pragma comment(lib, "Ws2_32.lib")

std::vector<SOCKET> clients;
std::unordered_map<SOCKET, std::string> clientNames;
std::mutex clientsMutex;

std::string FindReceiver(const std::string& str) {
    size_t pos1 = str.find('|');
    size_t pos2 = str.find('|', pos1 + 1);

    //sender = str.substr(0, pos1);
    std::string receiver = str.substr(pos1 + 1, pos2 - pos1 - 1);
    //std::string content = str.substr(pos2 + 1);

    return receiver;
}

std::string FindSender(const std::string& str) {
    size_t pos1 = str.find('|');
    size_t pos2 = str.find('|', pos1 + 1);

    std::string sender = str.substr(0, pos1);
    //std::string receiver = str.substr(pos1 + 1, pos2 - pos1 - 1);
    //std::string content = str.substr(pos2 + 1);

    return sender;
}

int server() {
    // Step 1: Initialize WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Step 2: Create a socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Step 3: Bind the socket
    sockaddr_in server_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(65432);  // Server port
    server_address.sin_addr.s_addr = INADDR_ANY; // Accept connections on any IP address

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Step 4: Listen for incoming connections
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 65432..." << std::endl;

    // Step 5: Accept a connection
    sockaddr_in client_address = {};
    int client_address_len = sizeof(client_address);
    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_address, &client_address_len);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "Accepted connection from " << client_ip << ":" << ntohs(client_address.sin_port) << std::endl;

    // Step 6: Communicate with the client
    char buffer[1024] = { 0 };
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::cout << "Received: " << buffer << std::endl;

        // Reverse the string
        std::string response(buffer);
        //std::reverse(response.begin(), response.end());

        // Send the reversed string back
        send(client_socket, response.c_str(), static_cast<int>(response.size()), 0);
        std::cout << "Reversed string sent back to client." << std::endl;
    }

    // Step 7: Clean up
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}


int server_loop() {
    // Step 1: Initialize WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Step 2: Create a socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Step 3: Bind the socket
    sockaddr_in server_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(65432);  // Server port
    server_address.sin_addr.s_addr = INADDR_ANY; // Accept connections on any IP address

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Step 4: Listen for incoming connections
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 65432..." << std::endl;

    // Step 5: Accept a connection
    sockaddr_in client_address = {};
    int client_address_len = sizeof(client_address);
    SOCKET client_socket = accept(server_socket, (sockaddr*)&client_address, &client_address_len);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
    std::cout << "Accepted connection from " << client_ip << ":" << ntohs(client_address.sin_port) << std::endl;

    bool stop = false;
    while (!stop) {
        // Step 6: Communicate with the client
        char buffer[1024] = { 0 };
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::cout << "Received: " << buffer << std::endl;

            // Reverse the string
            std::string response(buffer);
            if (response == "!bye") stop = true;

            //std::reverse(response.begin(), response.end());

            // Send the reversed string back
            send(client_socket, response.c_str(), static_cast<int>(response.size()), 0);
            std::cout << "Reversed string sent back to client." << std::endl;
        }
    }
    std::cout << "Closing connection\n";

    // Step 7: Clean up
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
    return 0;

}

void broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(clientsMutex);
    for (SOCKET s : clients) {
        send(s, message.c_str(), static_cast<int>(message.size()), 0);
    }
}

void updateUserList() {
    std::string userList = "System|all|";
    for (const auto& pair : clientNames) {
        userList += pair.second + ",";
    }
    broadcastMessage(userList);
}

void doClient(SOCKET client_socket, int connection) {
    bool stop = false;
    char buffer[1024] = { 0 };
    int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_received < 0) {
        closesocket(client_socket);
        return;
    }
    buffer[bytes_received] = '\0';
    std::string username = buffer;
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clientNames[client_socket] = username;
    }
    std::cout << username + " has entered the chatroom!"<<std::endl;
    broadcastMessage("Notification|all|" + username + " has entered the chatroom!");
	//send a new user list to everyone
    /*std::string userList = "System|all|";
    for (const auto& pair : clientNames) {
		userList += pair.second + ",";
    }
    std::cout << userList << std::endl;
    broadcastMessage(userList);*/
	updateUserList();

    while (!stop) {
        // Step 6: Communicate with the client
        //char buffer[1024] = { 0 };
        int bytes_received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received > 0) {
            buffer[bytes_received] = '\0';
            std::cout << connection;
            std::cout << " : Received: " << buffer << std::endl;

            // Send the reversed string back
			std::string sentence(buffer);
			std::string receiver = FindReceiver(sentence);
			std::string sender = FindSender(sentence);
            if (receiver == "all") {
                broadcastMessage(sentence);
            }
            else
            {
                //dm
                //std::lock_guard<std::mutex> lock(clientsMutex);
                for (const auto& pair : clientNames) {
                    if (pair.second == receiver || pair.second == sender) {
                        send(pair.first, sentence.c_str(), static_cast<int>(sentence.size()), 0);
                    }
				}
            }
            //std::string response = username + ": " + buffer;
            if (receiver == "!bye") stop = true;

            //std::reverse(response.begin(), response.end());
            
            if (stop == true) {
                std::string notification = "Notification|all|" + username + " has left the chatroom.";
                broadcastMessage(notification);
                clientNames.erase(client_socket);
                clients.erase(
                    std::remove(clients.begin(), clients.end(), client_socket),
                    clients.end()
                );
                updateUserList();
            }
            //for (SOCKET s : clients) {
            //    send(s, response.c_str(), static_cast<int>(response.size()), 0);
            //    std::cout << "Message sent to everyone " << std::endl;
            //    //tell everyone that this client is leaving
            //    if (stop == true) {
            //        std::string notification = "Client " + std::to_string(connection) + " has left the chatroom.";
            //        std::cout << notification << std::endl;
            //        send(s, notification.c_str(), static_cast<int>(notification.size()), 0);
            //    }
            //}
            //send(client_socket, response.c_str(), static_cast<int>(response.size()), 0);
            
        }
    }
	/*std::string notification = "Client " + std::to_string(connection) + " has left the chatroom.";
    for (SOCKET s : clients) {
        send(s, notification.c_str(), static_cast<int>(notification.size()), 0);
        std::cout << "Message sent to everyone " << std::endl;
    }*/
    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        clients.erase(
            std::remove(clients.begin(), clients.end(), client_socket),
            clients.end()
        );
    }
    std::cout << "Closing connection to client " << connection << ": " << username << "\n";
    closesocket(client_socket);
}


int server_loop_multi() {
    // Step 1: Initialize WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // Step 2: Create a socket
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_socket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed with error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // Step 3: Bind the socket
    sockaddr_in server_address = {};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(65432);  // Server port
    server_address.sin_addr.s_addr = INADDR_ANY; // Accept connections on any IP address

    if (bind(server_socket, (sockaddr*)&server_address, sizeof(server_address)) == SOCKET_ERROR) {
        std::cerr << "Bind failed with error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // Step 4: Listen for incoming connections
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed with error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server is listening on port 65432..." << std::endl;

    // Step 5: Accept a connection
    int connection = 0;
    while (1) { // server keeps running
        sockaddr_in client_address = {};
        int client_address_len = sizeof(client_address);
        SOCKET client_socket = accept(server_socket, (sockaddr*)&client_address, &client_address_len);
        if (client_socket == INVALID_SOCKET) {
            std::cerr << "Accept failed with error: " << WSAGetLastError() << std::endl;
            closesocket(server_socket);
            WSACleanup();
            return 1;
        }

        //client connect successfully
        std::lock_guard<std::mutex> lock(clientsMutex);
		clients.push_back(client_socket);//add to clients list
        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_address.sin_addr, client_ip, INET_ADDRSTRLEN);
        std::cout << "Accepted connection from " << client_ip << ":" << ntohs(client_address.sin_port);
        std::cout << " . Connection id = " << ++connection<<std::endl;

        std::thread* t = new std::thread(doClient, client_socket, connection);
        //    std::unique_ptr<std::thread> t = std::make_unique<std::thread>(doClient, client_socket, connection); 
         //   t->detach(); // probably best to free up resources by detaching
          // consider using a vector in order to keep all threads and free them eventually
            //   doClient(client_socket); 
             //  t.join();         
    }
    // Step 7: Clean up
    std::cout << "Server closed\n";
    closesocket(server_socket);
    WSACleanup();
    return 0;

}

int main() {
    // server();
    // server_loop(); 
    server_loop_multi();
    return 0;
}
