#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>
#include <vector>
#include <map>

#define IP "127.0.0.1"
#define prompt "% "
using namespace std;

class BBS_server
{
private:
    //db, mb
    map<string, string> db;

public:
    BBS_server(/* args */);
    ~BBS_server();
    int user_state = 0;
    string user;
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
    db.clear();
}

string BBS_server::Split_cmd(string cmd)
{
    string error("");
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
    string error("");
    if (cmd_list[0] == "register")
    {
        return Register(cmd_list);
    }
    else if (cmd_list[0] == "login")
    {
        return Login(cmd_list);
    }
    else if (cmd_list[0] == "logout")
    {
        return Logout();
    }
    else if (cmd_list[0] == "whoami")
    {
        return Whoami();
    }
    else if (cmd_list[0] == "list-user")
    {
        return List_user();
    }
    else if (cmd_list[0] == "exit")
    {
        return cmd_list[0];
    }
    return error;
}

string BBS_server::Register(vector<string> cmd_list)
{
    if (cmd_list.size() != 3)
    {
        return "Usage: register <username> <password>\n";
    }
    string username = cmd_list[1];
    string password = cmd_list[2];
    pair<map<string, string>::iterator, bool> ret;
    ret = db.insert(pair<string, string>(username, password));
    if (ret.second == true)
    {
        return "Register successfully.\n";
    }
    else
    {
        return "Username is already used.\n";
    }
}

string BBS_server::List_user()
{
    string list;
    for (auto it = db.begin(); it != db.end(); it++)
    {
        list.append((*it).first + "\n");
    }
    return list;
}

string BBS_server::Login(vector<string> cmd_list)
{
    if (cmd_list.size() != 3)
    {
        return "Usage: login <username> <password>\n";
    }
    if (user_state == 1)
    {
        return "Please logout first.\n";
    }
    map<string, string>::iterator iter;
    iter = db.find(cmd_list[1]);
    if (iter != db.end())
    {
        //user exist
        if (cmd_list[2] == iter->second)
        {
            //login successfully
            user_state = 1;
            user = iter->first;
            return "Welcome, " + iter->first + ".\n";
        }
        else
        {
            return "Login failed.\n";
        }
    }
    else
    {
        return "Login failed.\n";
    }
}

string BBS_server::Logout()
{
    if (user_state == 0)
    {
        return "Please login first.\n";
    }
    else
    {
        user_state = 0;
        return "Bye, " + user + ".\n";
    }
}

string BBS_server::Whoami()
{
    if (user_state == 0)
    {
        return "Please login first.\n";
    }
    else
    {
        return user + "\n";
    }
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
        perror("socket");
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
        //cout << "waiting for connect" << endl;
        if ((newfd = accept(sockfd, (struct sockaddr *)&new_addr, (socklen_t *)&addrlen)) < 0)
        {
            cout << "accept failed" << endl;
            perror("accept");
            return 0;
        }
        //cout << "new connection, socket fd: " << newfd << endl;
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
            if (r == 0)
            {
                //cout << "receive fail" << endl;
                string end_newfd = bbs.Split_cmd("exit");
                break;
            }
            string cmd(buff);
            //parase cmd to reply
            string reply = bbs.Split_cmd(cmd);
            if (reply == "exit")
            {
                if (bbs.user_state == 1)
                {
                    string bye_msg = "Bye, " + bbs.user + ".\n";
                    send(newfd, (char *)bye_msg.c_str(), bye_msg.length() * sizeof(bye_msg[0]), 0);
                }
                break;
            }
            //cout << "send reply" << endl;
            send(newfd, (char *)reply.c_str(), reply.length() * sizeof(reply[0]), 0);
            //cout << "send complete" << endl;
        }
        //close client fd
        bbs.user_state = 0;
        bbs.user = "";
        close(newfd);
    }

    return 0;
}
