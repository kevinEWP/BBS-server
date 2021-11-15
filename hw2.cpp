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
#include <sys/time.h>

#define IP "127.0.0.1"
#define prompt "% "
using namespace std;

class Message
{
public:
    string sender, messages;
    Message(string s, string m)
    {
        sender = s;
        messages = m;
    }
};

class Mailbox
{
private:
    /* data */
public:
    string owner;
    map<string, int> sender_list;
    vector<Message> msg_list;
    Mailbox(string user);
};

Mailbox::Mailbox(string user)
{
    owner = user;
}

class BBS_server
{
private:
    //db
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
    vector<Mailbox> user_list;
    string Send(vector<string> cmd_list);
    string List_msg();
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
    user_list.clear();
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
        int split = 1, send_c = 0;
        //cout << "cmd: " << cmd;
        ss << cmd;
        while (ss >> out)
        {
            if (out == "send")
            {
                send_c = 2;
            }
            if (send_c == split)
            {
                cmd_list.push_back(out);
                string msg_content, gg;
                getline(ss, gg, '\"'); //put space
                getline(ss, msg_content, '\"');
                cmd_list.push_back(msg_content);
                break;
            }
            cmd_list.push_back(out);
            split++;
        }
        ss.clear();
        /*
        for (int i = 0; i < cmd_list.size(); i++)
        {
            cout << cmd_list[i] << endl;
        }*/
    }
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
    else if (cmd_list[0] == "send")
    {
        return Send(cmd_list);
    }
    else if (cmd_list[0] == "list-msg")
    {
        return List_msg();
    }
    else if (cmd_list[0] == "receive")
    {
        return Receive(cmd_list);
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
        Mailbox mb(username);
        user_list.push_back(mb);
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

string BBS_server::Send(vector<string> cmd_list)
{
    if (cmd_list.size() != 3)
    {
        return "Usage: send <username> <message>\n";
    }
    if (user_state == 0)
    {
        return "Please login first.\n";
    }
    map<string, string>::iterator iter;
    iter = db.find(cmd_list[1]);
    if (iter != db.end())
    {
        //receiver exist
        //find receiver in the list
        for (int i = 0; i < user_list.size(); i++)
        {
            if (user_list[i].owner == cmd_list[1])
            {
                //check if user send to receiver already
                map<string, int>::iterator it;
                it = user_list[i].sender_list.find(user);
                if (it != user_list[i].sender_list.end())
                {
                    //send before, mail count+1
                    int mail_count = it->second;
                    mail_count++;
                    user_list[i].sender_list[user] = mail_count;
                }
                else
                {
                    user_list[i].sender_list[user] = 1;
                }
                //store msg into msg_list
                Message msg(user, cmd_list[2]);
                user_list[i].msg_list.push_back(msg);
                break;
            }
        }
        //cout << "store in mailbox" << endl;
        return "";
    }
    else
    {
        return "User not existed.\n";
    }
}

string BBS_server::List_msg()
{
    if (user_state == 0)
    {
        return "Please login first.\n";
    }
    //find your mailbox
    for (int i = 0; i < user_list.size(); i++)
    {
        if (user_list[i].owner == user)
        {
            if (user_list[i].msg_list.empty())
            {
                return "Your message box is empty.\n";
            }
            //list all sender
            string list, count;
            for (auto it = user_list[i].sender_list.begin(); it != user_list[i].sender_list.end(); it++)
            {
                count = to_string((*it).second);
                if ((*it).second == 0)
                {
                    continue;
                }
                list.append(count + " message from " + (*it).first + ".\n");
            }
            return list;
        }
    }
    return "";
}

string BBS_server::Receive(vector<string> cmd_list)
{
    if (cmd_list.size() != 2)
    {
        return "Usage: receive <username>\n";
    }
    if (user_state == 0)
    {
        return "Please login first.\n";
    }
    map<string, string>::iterator iter;
    iter = db.find(cmd_list[1]);
    if (iter != db.end())
    {
        //sender exist
        //find your mailbox
        for (int i = 0; i < user_list.size(); i++)
        {
            if (user_list[i].owner == user)
            {
                //list all sender
                for (auto it = user_list[i].msg_list.begin(); it != user_list[i].msg_list.end(); it++)
                {
                    if ((*it).sender == cmd_list[1])
                    {
                        string msg = (*it).messages;
                        int c = user_list[i].sender_list[cmd_list[1]];
                        user_list[i].msg_list.erase(it);
                        c--;
                        user_list[i].sender_list[cmd_list[1]] = c;
                        return msg + "\n";
                    }
                }
                return "";
            }
        }
    }
    else
    {
        return "User not existed.\n";
    }
    return "";
}

int main(int argc, char const *argv[])
{
    int sockfd, newfd;
    //max client 10
    int max_client = 10, client_socket[10], max_fd, sd;
    for (int i = 0; i < max_client; i++)
    {
        client_socket[i] = 0;
    }
    fd_set read_fd;
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
        FD_ZERO(&read_fd);
        FD_SET(sockfd, &read_fd);
        max_fd = sockfd;
        for (int i = 0; i < max_client; i++)
        {
            sd = client_socket[i];
            if (sd > 0)
            {
                FD_SET(sd, &read_fd);
            }
            if (sd > max_fd)
            {
                max_fd = sd;
            }
        }
        int client_req = select(max_fd + 1, &read_fd, NULL, NULL, NULL);
        if (client_req < 0)
        {
            cout << "select error" << endl;
        }
        //cout << "waiting for connect" << endl;
        //receive new connect request
        if (FD_ISSET(sockfd, &read_fd))
        {
            //a new connection request
            if ((newfd = accept(sockfd, (struct sockaddr *)&new_addr, (socklen_t *)&addrlen)) < 0)
            {
                cout << "accept failed" << endl;
                perror("accept");
                return 0;
            }
            //cout << "new connection, socket fd: " << newfd << endl;
            for (int i = 0; i < max_client; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = newfd;
                    cout << "Adding to list of sockets as " << i << endl;
                    break;
                }
            }
            //greeting
            send(newfd, greeting1, strlen(greeting1), 0);
            send(newfd, greeting2, strlen(greeting1), 0);
            send(newfd, greeting1, strlen(greeting1), 0);
            //send %
            send(newfd, (char *)"% ", strlen("% "), 0);
        }
        //connected client send requset
        for (int i = 0; i < max_client; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET(sd, &read_fd))
            {
                cout << "request from sd: " << sd << endl;
                //receive cmd from client
                char buff[1024];
                memset(buff, 0, 1024);
                auto r = recv(sd, (char *)buff, 1024, 0);
                if (r == 0)
                {
                    //cout << "receive fail" << endl;
                    //close client fd
                    bbs.user_state = 0;
                    bbs.user = "";
                    client_socket[i] = 0;
                    close(sd);
                }
                else
                {
                    string cmd(buff);
                    //parase cmd to reply
                    string reply = bbs.Split_cmd(cmd);
                    if (reply == "exit")
                    {
                        if (bbs.user_state == 1)
                        {
                            string bye_msg = "Bye, " + bbs.user + ".\n";
                            send(sd, (char *)bye_msg.c_str(), bye_msg.length() * sizeof(bye_msg[0]), 0);
                        }
                        //close client fd
                        bbs.user_state = 0;
                        bbs.user = "";
                        client_socket[i] = 0;
                        close(sd);
                    }
                    else
                    {
                        //cout << "send reply" << endl;
                        send(sd, (char *)reply.c_str(), reply.length() * sizeof(reply[0]), 0);
                        //send %
                        send(sd, (char *)"% ", strlen("% "), 0);
                        //cout << "send complete" << endl;
                    }
                }
            }
        }
    }

    return 0;
}
