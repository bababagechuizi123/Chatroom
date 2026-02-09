#ifndef NETWORK_H
#define NETWORK_H

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include<mutex>
#include<vector>

class Message {
public:
	std::string content;
	std::string sender;
	std::string receiver;

	Message(std:: string _content, std::string _sender, std::string _receiver): content(_content), sender(_sender), receiver(_receiver){}
};

class Client {
public:
	SOCKET client_socket;
	std::vector<Message> msgs;
	std::mutex msgMutex;
	std::thread recv_thread;
	std::string username;
	std::string userList;
	WSADATA wsaData;
	bool newuser = false;

	Client();
	~Client();

	void recvThread(SOCKET sock);

	void connect_to_server(const std::string& username);

	std::string updateUserList();

	void sendMessage(const std::string& msg, std::string receiver);

	std::vector<Message> getMessage();

	std::vector<Message> getPrivateMessage(const std::string& user1, const std::string& user2);

	bool isNewUser() { return newuser; }

	void disconnect();
};


#endif
