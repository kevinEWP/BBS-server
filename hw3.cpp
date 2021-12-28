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
#define MAXLEN 4096
using namespace std;

//version 1 struct
struct head
{
    unsigned char flag;
    unsigned char version;
    unsigned char payload[0];
};

struct record
{
    unsigned short len;
    unsigned char data[0];
};

//version 2 base64
static char SixBitToChar(char b)
{
    char lookupTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M',
        'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm',
        'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

    if ((b >= 0) && (b <= 63))
    {
        return lookupTable[(int)b];
    }
    else
    {
        return ' ';
    }
}

static string Encode(string data)
{
    int length, length2;
    int blockCount;
    int paddingCount;

    length = data.length();

    if ((length % 3) == 0)
    {
        paddingCount = 0;
        blockCount = length / 3;
    }
    else
    {
        paddingCount = 3 - (length % 3);
        blockCount = (length + paddingCount) / 3;
    }

    length2 = length + paddingCount;

    char *source2;
    source2 = new char[length2];

    for (int x = 0; x < length2; x++)
    {
        if (x < length)
        {
            source2[x] = data[x];
        }
        else
        {
            source2[x] = 0;
        }
    }

    char b1, b2, b3;
    char temp, temp1, temp2, temp3, temp4;
    char *buffer = new char[blockCount * 4];
    string result;

    for (int x = 0; x < blockCount; x++)
    {
        b1 = source2[x * 3];
        b2 = source2[x * 3 + 1];
        b3 = source2[x * 3 + 2];

        temp1 = (char)((b1 & 252) >> 2);

        temp = (char)((b1 & 3) << 4);
        temp2 = (char)((b2 & 240) >> 4);
        temp2 += temp;

        temp = (char)((b2 & 15) << 2);
        temp3 = (char)((b3 & 192) >> 6);
        temp3 += temp;

        temp4 = (char)(b3 & 63);

        buffer[x * 4] = temp1;
        buffer[x * 4 + 1] = temp2;
        buffer[x * 4 + 2] = temp3;
        buffer[x * 4 + 3] = temp4;
    }

    for (int x = 0; x < blockCount * 4; x++)
    {
        result += SixBitToChar(buffer[x]);
    }

    switch (paddingCount)
    {
    case 0:
        break;
    case 1:
        result[blockCount * 4 - 1] = '=';
        break;
    case 2:
        result[blockCount * 4 - 1] = '=';
        result[blockCount * 4 - 2] = '=';
        break;
    default:
        break;
    }

    delete[] source2;
    delete[] buffer;

    return result;
}

static char CharToSixBit(char c)
{
    char lookupTable[] = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N',
        'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
        'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '+', '/'};

    if (c == '=')
    {
        return 0;
    }
    else
    {
        for (int x = 0; x < 64; x++)
        {
            if (lookupTable[x] == c)
                return (char)x;
        }

        return 0;
    }
}

static string Decode(string data)
{
    int length, length2, length3;
    int blockCount;
    int paddingCount = 0;
    int dataLength = data.length();

    length = dataLength;
    blockCount = length / 4;
    length2 = blockCount * 3;

    for (int x = 0; x < 2; x++)
    {
        if (data[length - x - 1] == '=')
            paddingCount++;
    }

    char *buffer = new char[length];
    char *buffer2 = new char[length2];

    for (int x = 0; x < length; x++)
    {
        buffer[x] = CharToSixBit(data[x]);
    }

    char b, b1, b2, b3;
    char temp1, temp2, temp3, temp4;

    for (int x = 0; x < blockCount; x++)
    {
        temp1 = buffer[x * 4];
        temp2 = buffer[x * 4 + 1];
        temp3 = buffer[x * 4 + 2];
        temp4 = buffer[x * 4 + 3];

        b = (char)(temp1 << 2);
        b1 = (char)((temp2 & 48) >> 4);
        b1 += b;

        b = (char)((temp2 & 15) << 4);
        b2 = (char)((temp3 & 60) >> 2);
        b2 += b;

        b = (char)((temp3 & 3) << 6);
        b3 = temp4;
        b3 += b;

        buffer2[x * 3] = b1;
        buffer2[x * 3 + 1] = b2;
        buffer2[x * 3 + 2] = b3;
    }

    length3 = length2 - paddingCount;
    string result;

    for (int x = 0; x < length3; x++)
    {
        result += buffer2[x];
    }

    delete[] buffer;
    delete[] buffer2;

    return result;
}

class ChatMember
{
public:
    string username;
    int online;
    int tcpcli;
    struct sockaddr_in udpsock;
    int version;
    int warnnig;
    ChatMember()
    {
        online = 0;
        version = 0;
        tcpcli = 0;
        warnnig = 0;
    }
};

class Filterlist
{
public:
    vector<string> filter_words;
    vector<string> ban_words; //alternative words
    int violate;
    string violator;
    Filterlist()
    {
        violate = 0;
        filter_words.push_back("how");
        filter_words.push_back("you");
        filter_words.push_back("or");
        filter_words.push_back("pek0");
        filter_words.push_back("tea");
        filter_words.push_back("ha");
        filter_words.push_back("kon");
        filter_words.push_back("pain");
        filter_words.push_back("Starburst Stream");
        ban_words.push_back("***");
        ban_words.push_back("***");
        ban_words.push_back("**");
        ban_words.push_back("****");
        ban_words.push_back("***");
        ban_words.push_back("**");
        ban_words.push_back("***");
        ban_words.push_back("****");
        ban_words.push_back("****************");
    }
    string Check_filter(string input, string person)
    {

        size_t found;
        for (int i = 0; i < 9; i++)
        {
            int pos = 0;
            while (pos < input.size())
            {
                found = input.find(filter_words[i], pos);
                if (found != string::npos)
                {
                    violate = 1;
                    violator = person;
                    input.replace(found, ban_words[i].size(), ban_words[i]);
                    pos = found + ban_words.size();
                }
                else
                {
                    break;
                }
            }
        }
        //cout << input << endl;
        return input;
    }
    void SetZero()
    {
        violate = 0;
        violator = "";
    }
};

class Blacklist
{
public:
    vector<string> banned_users;
    bool Check_ban(string username)
    {
        if (banned_users.empty())
        {
            return false;
        }
        for (auto it = banned_users.begin(); it != banned_users.end(); it++)
        {
            if (username == *it)
            {
                return true;
            }
        }
        return false;
    }
};

class ChatHistory
{
public:
    string history;
    string latesetname, latestmsg;
    uint16_t namelen, msglen;
    //char debugname[256];
    void Addrecord(string name, string message)
    {
        history.append(name + ":" + message + "\n");
        latesetname = name;
        latestmsg = message;
    }
};

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
    void CloseClient(int sd, int uid)
    {
        // close client fd
        user_state[uid] = 0;
        roommember[uid].username = "";
        roommember[uid].tcpcli = 0;
        roommember[uid].online = 0;
        close(sd);
    }
    string Split_cmd(string cmd, int uid);
    string Parase(vector<string> cmd_list, int uid);
    string Register(vector<string> cmd_list);
    string Login(vector<string> cmd_list, int uid);
    string Logout(vector<string> cmd_list, int uid);
    ///////////////////////////////HW3
    vector<ChatMember> roommember;
    Filterlist no_words;
    Blacklist banned;
    ChatHistory chat;
    void BindRoom(int port, int version, int uid);
    string EnterRoom(string cmd, string p, string v, int uid);
    int Chat_handler(unsigned char *rcvmsg);
    void SendtoAll(unsigned char *sendbuf, int version);
};

BBS_server::BBS_server(int max_client)
{
    for (int i = 0; i < max_client; i++)
    {
        ChatMember person;
        person.username = "";
        user_state.push_back(0);
        roommember.push_back(person);
    }
    maxuser = max_client;
}

BBS_server::~BBS_server()
{
    // clean db, mb
    db.clear();
    user_state.clear();
    roommember.clear();
}
//add new func
int isNumber(string str)
{
    for (char const &c : str)
    {
        if (std::isdigit(c) == 0)
            return -1;
    }
    return stoi(str);
}

uint16_t bodycpy(char *dest, char *src)
{
    size_t i;
    uint16_t len = 0;
    for (i = 0; src[i] != '\n'; i++)
    {
        dest[i] = src[i];
        len++;
    }

    return len;
}

string BBS_server::Split_cmd(string cmd, int uid)
{
    string error("");
    vector<string> cmd_list;
    if (cmd.length() < 4)
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
    else if (cmd_list[0] == "enter-chat-room")
    {
        if (cmd_list.size() != 3)
        {
            return "Usage: enter-chat-room <port> <version>\n";
        }
        return EnterRoom(cmd_list[0], cmd_list[1], cmd_list[2], uid);
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
            if (roommember[i].username == cmd_list[1])
            {
                return "Please logout first.\n";
            }
        }
    }
    map<string, string>::iterator iter;
    iter = db.find(cmd_list[1]);
    if (iter != db.end())
    {
        if (banned.Check_ban(iter->first))
        {
            return "We don't welcome " + iter->first + "!\n";
        }
        if (cmd_list[2] == iter->second)
        {
            // login successfully
            user_state[uid] = 1;
            roommember[uid].username = iter->first;
            return "Welcome, " + iter->first + ".\n";
        }
        else
        {
            //password wrong
            return "Login failed.\n";
        }
    }
    else
    {
        //user doesn't exist
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
        roommember[uid].online = 0;
        string owner = roommember[uid].username;
        roommember[uid].username = "";
        return "Bye, " + owner + ".\n";
    }
}

void BBS_server::BindRoom(int port, int version, int uid)
{
    //fill in roommember
    roommember[uid].online = 1;
    roommember[uid].version = version;
    roommember[uid].udpsock.sin_port = htons(port);
}

string BBS_server::EnterRoom(string cmd, string p, string v, int uid)
{
    int port, version;
    port = isNumber(p);
    version = isNumber(v);
    if (port < 1 || port > 65535)
    {
        return "Port " + p + " is not valid.\n ";
    }
    if (version != 1 && version != 2)
    {
        return "Version " + v + " is not supported.\n";
    }
    if (user_state[uid] == 0)
    {
        return "Please login first.\n";
    }
    //bind to chatroom
    BindRoom(port, version, uid);
    //success
    string welcomeMsg("Welcome to public chat room.\nPort:");
    welcomeMsg.append(to_string(port) + "\nVersion:" + to_string(version) + "\n" + chat.history);
    return welcomeMsg;
}

int BBS_server::Chat_handler(unsigned char *rcvmsg)
{
    string violator;
    struct head *info = (struct head *)rcvmsg;
    if (info->flag != 0x1)
    {
        return 0;
    }
    if (info->version == 0x1)
    {
        struct record *p1 = (struct record *)(rcvmsg + sizeof(struct head));
        uint16_t namelen = ntohs(p1->len);
        char name[namelen + 1];
        memset(name, 0, namelen + 1);
        memcpy(name, p1->data, namelen);
        struct record *p2 = (struct record *)(rcvmsg + sizeof(struct head) + sizeof(struct record) + namelen);
        uint16_t msglen = ntohs(p2->len);
        char msg[msglen + 1];
        memset(msg, 0, msglen + 1);
        memcpy(msg, p2->data, msglen);
        chat.namelen = namelen;
        chat.msglen = msglen;
        //filtering
        string input(msg), output;
        violator = name;
        output = no_words.Check_filter(input, violator);
        //add record to chathistory
        chat.Addrecord(violator, output);
    }
    else if (info->version == 0x2)
    {
        char namebuf[256], msgbuf[1024];
        memset(namebuf, 0, 256);
        memset(msgbuf, 0, 1024);
        char *p1 = (char *)(rcvmsg + sizeof(struct head));
        uint16_t namelen = bodycpy(namebuf, p1);
        //string name = namebuf;
        string sender = Decode(namebuf);
        char *p2 = (char *)(p1 + namelen + 1);
        uint16_t msglen = bodycpy(msgbuf, p2);
        string text = Decode(msgbuf);
        //filtering
        string output;
        output = no_words.Check_filter(text, sender);
        //add record to chathistory
        chat.Addrecord(sender, output);
    }
    //if violate the rule
    if (no_words.violate == 1)
    {
        for (auto i = 0; i < roommember.size(); i++)
        {
            if (roommember[i].username == no_words.violator)
            {
                //find you
                roommember[i].warnnig++;
                if (roommember[i].warnnig == 3)
                {
                    //add user into ban list
                    banned.banned_users.push_back(roommember[i].username);
                    //cond. logout user immediately
                    vector<string> byebye;
                    byebye.push_back("logout");
                    string bye_msg = Logout(byebye, i);
                    send(roommember[i].tcpcli, (char *)bye_msg.c_str(), bye_msg.length() * sizeof(bye_msg[0]), 0);
                    send(roommember[i].tcpcli, (char *)"% ", strlen("% "), 0);
                    //CloseClient(roommember[i].tcpcli, i);
                }
                break;
            }
        }
        no_words.SetZero();
    }
    return 1;
}

void BBS_server::SendtoAll(unsigned char *sendbuf, int version)
{
    string name(chat.latesetname), msg(chat.latestmsg);
    // uint16_t name_len = (uint16_t)strlen(name.c_str());
    // uint16_t mesg_len = (uint16_t)strlen(msg.c_str());
    uint16_t name_len = chat.namelen;
    uint16_t mesg_len = chat.msglen;
    if (version == 1)
    {
        struct head *info = (struct head *)sendbuf;
        struct record *p1 = (struct record *)(sendbuf + sizeof(struct head));
        struct record *p2 = (struct record *)(sendbuf + sizeof(struct head) + sizeof(struct record) + name_len);
        info->flag = 0x1;
        info->version = 0x1;
        p1->len = htons(name_len);
        memcpy(p1->data, (const char *)name.c_str(), name_len);
        //memcpy(p1->data, chat.debugname, name_len);
        p2->len = htons(mesg_len);
        memcpy(p2->data, (const char *)msg.c_str(), mesg_len);
        //chat
        chat.latesetname = "";
        chat.latestmsg = "";
    }
    else if (version == 2)
    {
        string sender = Encode(name), text = Encode(msg);
        sprintf((char *)sendbuf, "\x01\x02%s\n%s\n", (const char *)sender.c_str(), (const char *)text.c_str());
        //chat
        chat.latesetname = "";
        chat.latestmsg = "";
    }
}

int main(int argc, char const *argv[])
{
    char *greeting1 = (char *)"********************************\n";
    char *greeting2 = (char *)"** Welcome to the BBS server. **\n";
    if (argc != 2)
    {
        cout << "ussage: ./hw3 PORT" << endl;
        return 0;
    }
    // max client 10
    int listenfd, connectfd, udpfd;
    int max_client = 10, max_fd, sd;
    fd_set read_fd;
    ssize_t n;
    socklen_t addrlen;
    struct sockaddr_in servaddr, cliaddr;
    string port_num = argv[1];
    int PORT = stoi(port_num);
    //set up tcp socket
    const int on = 1;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return 0;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0)
    {
        perror("bind");
        cout << "bind" << endl;
        return 0;
    }
    cout << "listen on ip:" << IP << " port " << argv[1] << endl;
    if (listen(listenfd, 10) < 0)
    {
        perror("listen");
        return 0;
    }
    //set up udp socket
    if ((udpfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        return 0;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, IP, &servaddr.sin_addr);
    servaddr.sin_port = htons(PORT);
    if (bind(udpfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr)) < 0)
    {
        perror("bind");
        cout << "bind" << endl;
        return 0;
    }
    //activate bbs
    BBS_server bbs(max_client);
    while (true)
    {
        FD_ZERO(&read_fd);
        FD_SET(listenfd, &read_fd);
        FD_SET(udpfd, &read_fd);
        max_fd = max(listenfd, udpfd);
        for (int i = 0; i < max_client; i++)
        {
            sd = bbs.roommember[i].tcpcli;
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
        // receive new tcp connect request
        if (FD_ISSET(listenfd, &read_fd))
        {
            addrlen = sizeof(cliaddr);
            // a new connection request
            if ((connectfd = accept(listenfd, (struct sockaddr *)&cliaddr, (socklen_t *)&addrlen)) < 0)
            {
                cout << "accept failed" << endl;
                perror("accept");
                return 0;
            }
            // cout << "new connection, socket fd: " << newfd << endl;
            for (int i = 0; i < max_client; i++)
            {
                if (bbs.roommember[i].tcpcli == 0)
                {
                    bbs.roommember[i].tcpcli = connectfd;
                    bbs.roommember[i].udpsock = cliaddr;
                    // cout << "Adding to list of sockets as " << i << endl;
                    break;
                }
            }
            // greeting
            send(connectfd, greeting1, strlen(greeting1), 0);
            send(connectfd, greeting2, strlen(greeting2), 0);
            send(connectfd, greeting1, strlen(greeting1), 0);
            // send %
            send(connectfd, (char *)"% ", strlen("% "), 0);
        }
        //receive udp data
        if (FD_ISSET(udpfd, &read_fd))
        {
            addrlen = sizeof(cliaddr);
            unsigned char rcvbuff[MAXLEN], sendbuff[MAXLEN];
            memset(rcvbuff, 0, MAXLEN);
            memset(sendbuff, 0, MAXLEN);
            n = recvfrom(udpfd, rcvbuff, MAXLEN, 0, (struct sockaddr *)&cliaddr, &addrlen);
            cout << "receive chat\n";
            if (bbs.Chat_handler(rcvbuff))
            {
                for (int i = 0; i < max_client; i++)
                {
                    if (bbs.roommember[i].online == 1)
                    {
                        //send udp paket to online users
                        cout << "send udp paket to online users\n";
                        addrlen = sizeof(bbs.roommember[i].udpsock);
                        bbs.SendtoAll(sendbuff, bbs.roommember[i].version);
                        sendto(udpfd, sendbuff, n, 0, (struct sockaddr *)&bbs.roommember[i].udpsock, addrlen);
                        memset(sendbuff, 0, MAXLEN);
                    }
                }
            }
        }
        // connected tcp client send requset
        for (int i = 0; i < max_client; i++)
        {
            sd = bbs.roommember[i].tcpcli;
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
                    bbs.CloseClient(sd, i);
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
                        bbs.CloseClient(sd, i);
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
