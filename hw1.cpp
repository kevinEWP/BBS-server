#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define IP "127.0.0.1"
#define prompt "% "
using namespace std;

class BBS_server
{
private:
    //db, mb
public:
    BBS_server(/* args */);
    ~BBS_server();
    string Split_cmd(string cmd);
    string Parase(string cmd, string cmd_list);
    string Register();
    string Login();
    string Logout();
    string Whoami();
    string List_user();
    //Message Box
    string Send();
    string List_msg();
    string Receive();
};

BBS_server::BBS_server(/* args */)
{
    //load in db & mseeage box
    //db = args
    //mb = args
}

BBS_server::~BBS_server()
{
    //clean db, mb
}

int main(int argc, char const *argv[])
{
    int sockfd, newfd;
    string port_num = argv[1];
    int PORT = stoi(port_num);
    struct sockaddr_in addr;
    char *greeting1 = (char *)"********************************\n";
    char *greeting2 = (char *)"** Welcome to the BBS server. **\n";
    if (argc != 2)
    {
        cout << "ussage: ./hw1 PORT" << endl;
        return 0;
    }
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket():");
        return 0;
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    //addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_addr.s_addr = inet_addr(IP);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr)) < 0)
    {
        perror("bind");
        cout << "bind" << endl;
        return 0;
    }
    cout << "listen on ip:" << IP << " port " << argv[1] << endl;
    if (listen(sockfd, 10) < 0)
    {
        perror("listen");
        return 0;
    }

    struct sockaddr_in new_addr;
    int addrlen = sizeof(new_addr);
    while (true)
    {
        cout << "waiting for connect" << endl;
        if ((newfd = accept(sockfd, (struct sockaddr *)&new_addr, (socklen_t *)&addrlen)) < 0)
        {
            cout << "accept failed" << endl;
            perror("accept");
            return 0;
        }
        cout << "new connection, socket fd: " << newfd << endl;
        //greeting
        send(newfd, greeting1, strlen(greeting1), 0);
        send(newfd, greeting2, strlen(greeting1), 0);
        send(newfd, greeting1, strlen(greeting1), 0);
        //comunicate with client
        while (true)
        {
            //send %
            send(newfd, (char *)"% ", strlen("% "), 0);
            //receive cmd from client

            //parase cmd to reply

            //if reply == exit: break

            //if reply == other: send reply
        }
        //close client fd
        cout << "end of connection" << endl;
        close(newfd);
    }

    return 0;
}
