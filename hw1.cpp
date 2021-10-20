#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>

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
    string Parase(vector<string> cmd_list);
    string Register(vector<string> cmd_list);
    string Login(vector<string> cmd_list);
    string Logout();
    string Whoami();
    string List_user();
    //Message Box
    string Send(vector<string> cmd_list);
    string List_msg(vector<string> cmd_list);
    string Receive(vector<string> cmd_list);
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

string BBS_server::Split_cmd(string cmd)
{
    string error("invalid command\n");
    vector<string> cmd_list;
    if (cmd.length() < 5)
    {
        return error;
    }
    else
    {
        //cout << "start split" << endl;
        stringstream ss;
        string out;
        ss << cmd;
        while (ss >> out)
        {
            cmd_list.push_back(out);
            //cout << out << endl;
        }
        ss.clear();
    }
    //cout << "split !!!" << endl;
    return Parase(cmd_list);
}

string BBS_server::Parase(vector<string> cmd_list)
{
    string error("command not found\n");
    if (cmd_list[0] == "register")
    {
        //return Register(cmd_list);
        return error;
    }
    else if (cmd_list[0] == "login")
    {
        //return Login(cmd_list);
        return error;
    }
    else if (cmd_list[0] == "logout")
    {
        //return Logout();
        return error;
    }
    else if (cmd_list[0] == "whoami")
    {
        //return Whoami();
        return error;
    }
    else if (cmd_list[0] == "list-user")
    {
        //return List_user();
        return error;
    }
    else if (cmd_list[0] == "exit")
    {
        return cmd_list[0];
    }
    return error;
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
    BBS_server bbs;
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
            char buff[1024];
            memset(buff, 0, 1024);
            auto r = recv(newfd, (char *)buff, 1024, 0);
            //cout << "got command" << endl;
            if (r == 0)
            {
                cout << "receive fail" << endl;
            }
            string cmd(buff);
            //parase cmd to reply
            string reply = bbs.Split_cmd(cmd);
            if (reply == "exit")
            {
                break;
            }
            cout << "send reply" << endl;
            send(newfd, (char *)reply.c_str(), reply.length() * sizeof(reply[0]), 0);
            cout << "send complete" << endl;
        }
        //close client fd
        send(newfd, (char *)"bye~", strlen("bye~"), 0);
        cout << "end of connection" << endl;
        close(newfd);
    }

    return 0;
}
