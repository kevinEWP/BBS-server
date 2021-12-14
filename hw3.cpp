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
#include <ctime>

#define IP "127.0.0.1"
#define prompt "% "
using namespace std;

class BBS_server
{
private:
    // db username,password
    map<string, string> db;

public:
    BBS_server(int max_client);
    ~BBS_server();
    // diff from HW1 user_state, user(vec)
    int maxuser;
    vector<int> user_state; // login table
    vector<string> user;    // login username
    string Split_cmd(string cmd, int uid);
    string Parase(vector<string> cmd_list, int uid);
    string Register(vector<string> cmd_list);
    string Login(vector<string> cmd_list, int uid);
    string Logout(vector<string> cmd_list, int uid);
};

BBS_server::BBS_server(int max_client)
{
    for (int i = 0; i < max_client; i++)
    {
        user_state.push_back(0);
        user.push_back("");
    }
    maxuser = max_client;
    // cout << "Month: " << 1 + ltm->tm_mon << endl;
    // cout << "Day: " << ltm->tm_mday << endl;
}

BBS_server::~BBS_server()
{
    // clean db, mb
    db.clear();
    user_state.clear();
    user.clear();
}

string BBS_server::Split_cmd(string cmd, int uid)
{
    string error("");
    vector<string> cmd_list;
    if (cmd.length() < 5)
    {
        return error;
    }
    else
    {
        // cout << "start split" << endl;
        stringstream ss;
        string out;
        int split = 1, post_c = 0, create = 0, update = 0, comment = 0;
        // cout << "cmd: " << cmd;
        ss << cmd;
        while (ss >> out)
        {
            cmd_list.push_back(out);
            if (out == "create-post")
            {
                post_c = 2;
                create = 1;
            }
            if (out == "update-post")
            {
                post_c = 2;
                update = 1;
            }
            if (out == "comment")
            {
                post_c = 2;
                comment = 1;
            }
            if (post_c == split && create == 1)
            {
                // cmd:create-post
                string title, content, gg;
                if (ss.eof())
                {
                    break;
                }
                ss >> out;
                // cout << "f out --t or --con ? :" << out << endl;
                if (out == "--title")
                {
                    cmd_list.push_back(out);
                    ss >> out;
                    while (out != "--content")
                    {
                        title.append(out + " ");
                        ss >> out;
                        if (ss.eof())
                        {
                            break;
                        }
                    }
                    cmd_list.push_back(title);
                    // cout << "title= " << title << endl;
                    cmd_list.push_back(out);
                    getline(ss, gg, ' ');
                    getline(ss, content, '\n');
                    cmd_list.push_back(content);
                    break;
                }
                else if (out == "--content")
                {
                    cmd_list.push_back(out);
                    ss >> out;
                    while (out != "--title")
                    {
                        content.append(out + " ");
                        ss >> out;
                        if (ss.eof())
                        {
                            break;
                        }
                    }
                    cmd_list.push_back(content);
                    // cout << "content= " << content << endl;
                    cmd_list.push_back(out);
                    getline(ss, gg, ' ');
                    getline(ss, title, '\n');
                    cmd_list.push_back(title);
                    break;
                }
                else
                {
                    break;
                }
            }
            if (post_c == split && update == 1)
            {
                // cmd: update-post
                string modify, gg;
                ss >> out;
                cmd_list.push_back(out);
                getline(ss, gg, ' ');
                getline(ss, modify, '\n');
                cmd_list.push_back(modify);
                break;
            }
            if (post_c == split && comment == 1)
            {
                // cmd: comment
                string text, gg;
                getline(ss, gg, ' ');
                getline(ss, text, '\n');
                cmd_list.push_back(text);
                break;
            }
            ++split;
        }
        ss.str("");
        ss.clear();
        /*
        //cout << "cmd list size: " << cmd_list.size() << endl;
        for (int i = 0; i < cmd_list.size(); i++)
        {
            cout << cmd_list[i] << endl;
        }*/
    }
    return Parase(cmd_list, uid);
}

string BBS_server::Parase(vector<string> cmd_list, int uid)
{
    string error("");
    if (cmd_list[0] == "register")
    {
        return Register(cmd_list);
    }
    else if (cmd_list[0] == "login")
    {
        return Login(cmd_list, uid);
    }
    else if (cmd_list[0] == "logout")
    {
        return Logout(cmd_list, uid);
    }
    else if (cmd_list[0] == "exit")
    {
        if (cmd_list.size() != 1)
        {
            return "Usage: exit\n";
        }
        else
        {
            return cmd_list[0];
        }
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

string BBS_server::Login(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 3)
    {
        return "Usage: login <username> <password>\n";
    }
    if (user_state[uid] == 1)
    {
        // already login
        return "Please logout first.\n";
    }
    for (int i = 0; i < maxuser; i++)
    {
        if (user_state[i] == 1)
        {
            // other login user
            if (user[i] == cmd_list[1])
            {
                return "Please logout first.\n";
            }
        }
    }
    map<string, string>::iterator iter;
    iter = db.find(cmd_list[1]);
    if (iter != db.end())
    {
        // user exist
        if (cmd_list[2] == iter->second)
        {
            // login successfully
            user_state[uid] = 1;
            user[uid] = iter->first;
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

string BBS_server::Logout(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 1)
    {
        return "Usage: logout\n";
    }
    if (user_state[uid] == 0)
    {
        return "Please login first.\n";
    }
    else
    {
        user_state[uid] = 0;
        string owner = user[uid];
        user[uid] = "";
        return "Bye, " + owner + ".\n";
    }
}

int main(int argc, char const *argv[])
{
    int sockfd, newfd;
    // max client 10
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
    // addr.sin_addr.s_addr = htonl(INADDR_ANY);
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
    BBS_server bbs(max_client);
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
        // cout << "waiting for connect" << endl;
        // receive new connect request
        if (FD_ISSET(sockfd, &read_fd))
        {
            // a new connection request
            if ((newfd = accept(sockfd, (struct sockaddr *)&new_addr, (socklen_t *)&addrlen)) < 0)
            {
                cout << "accept failed" << endl;
                perror("accept");
                return 0;
            }
            // cout << "new connection, socket fd: " << newfd << endl;
            for (int i = 0; i < max_client; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = newfd;
                    // cout << "Adding to list of sockets as " << i << endl;
                    break;
                }
            }
            // greeting
            send(newfd, greeting1, strlen(greeting1), 0);
            send(newfd, greeting2, strlen(greeting2), 0);
            send(newfd, greeting1, strlen(greeting1), 0);
            // send %
            send(newfd, (char *)"% ", strlen("% "), 0);
        }
        // connected client send requset
        for (int i = 0; i < max_client; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET(sd, &read_fd))
            {
                // cout << "request from sd: " << sd << endl;
                // receive cmd from client
                char buff[1024];
                memset(buff, 0, 1024);
                auto r = recv(sd, (char *)buff, 1024, 0);
                if (r == 0)
                {
                    // cout << "receive fail" << endl;
                    // close client fd
                    bbs.user_state[i] = 0;
                    bbs.user[i] = "";
                    client_socket[i] = 0;
                    close(sd);
                }
                else
                {
                    string cmd(buff);
                    // parase cmd to reply
                    string reply = bbs.Split_cmd(cmd, i);
                    if (reply == "exit")
                    {
                        //excute exit
                        if (bbs.user_state[i] == 1)
                        {
                            //user not logged out
                            vector<string> byebye;
                            byebye.push_back("logout");
                            string bye_msg = bbs.Logout(byebye, i);
                            send(sd, (char *)bye_msg.c_str(), bye_msg.length() * sizeof(bye_msg[0]), 0);
                        }
                        // close client fd
                        bbs.user_state[i] = 0;
                        bbs.user[i] = "";
                        client_socket[i] = 0;
                        close(sd);
                    }
                    else
                    {
                        // cout << "send reply" << endl;
                        send(sd, (char *)reply.c_str(), reply.length() * sizeof(reply[0]), 0);
                        // send %
                        send(sd, (char *)"% ", strlen("% "), 0);
                        // cout << "send complete" << endl;
                    }
                }
            }
        }
    }

    return 0;
}
