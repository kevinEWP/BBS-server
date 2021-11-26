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
//HW2
class Comment
{
public:
    string user, comments;
    Comment(string owner, string text)
    {
        user = owner;
        comments = text;
    }
};
class Post
{
private:
    /* data */
public:
    int s_n;
    string board_name, title, author, date;
    vector<string> contents;
    vector<Comment> comments;
    Post(int n, string user, string time, string board, string headline);
    void CreateContent(string text);
    void CreateComment(Comment newcomment);
    string ShowPost();
    void UpdatePost(string which_update, string newthing);
    ~Post();
};
Post::Post(int n, string user, string time, string board, string headline)
{
    s_n = n;
    author = user;
    date = time;
    board_name = board;
    title = headline;
}
Post::~Post()
{
    contents.clear();
    comments.clear();
}
void Post::CreateContent(string text)
{
    //if text has <br>, save as next line
    string cut, trash;
    stringstream handler;
    handler << text;
    while (!handler.eof())
    {
        getline(handler, cut, '<');
        getline(handler, trash, '>');
        cout << cut << endl;
        contents.push_back(cut);
    }
}
void Post::CreateComment(Comment newcomment)
{
    comments.push_back(newcomment);
}
string Post::ShowPost()
{
    //Show the post cmd: read <post-S/N>
    string show, line("--\n");
    show.append("Author: " + author + "\n");
    show.append("Title: " + title + "\n");
    show.append("Date: " + date + "\n");
    show.append(line);
    for (int i = 0; i < contents.size(); i++)
    {
        show.append(contents[i] + "\n");
    }
    show.append(line);
    if (!comments.empty())
    {
        for (int i = 0; i < comments.size(); i++)
        {
            show.append(comments[i].user + ": " + comments[i].comments + "\n");
        }
    }
    return show;
}
void Post::UpdatePost(string which_update, string newthing)
{
    if (which_update == "--title")
    {
        title = newthing;
    }
    if (which_update == "--content")
    {
        contents.clear();
        string cut, trash;
        stringstream handler;
        handler << newthing;
        while (!handler.eof())
        {
            getline(handler, cut, '<');
            getline(handler, trash, '>');
            cout << cut << endl;
            contents.push_back(cut);
        }
    }
}

class Board
{
private:
    /* data */
public:
    int index;
    string board_name, moderater;
    vector<Post> posts;
    Board(string name, string user);
    void CreatePost(Post newpost);
    string ShowBoard();
    ~Board();
};

Board::Board(string name, string user)
{
    board_name = name;
    moderater = user;
}

Board::~Board()
{
    posts.clear();
}
void Board::CreatePost(Post newpost)
{
    posts.push_back(newpost);
}
string Board::ShowBoard()
{
    //List all posts in a board,cmd:list-post <board-name>
    string table = "S/N Title Author Date\n";
    string list(table);
    for (int i = 0; i < posts.size(); i++)
    {
        list.append(to_string(posts[i].s_n) + " " + posts[i].title + " " + posts[i].author + " " + posts[i].date + "\n");
    }
    return list;
}

//HW1
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
    //db username,password
    map<string, string> db;
    map<string, Board> board_list;
    vector<Board> board_index;
    map<int, Post> post_list;

public:
    BBS_server(int max_client);
    ~BBS_server();
    //diff from HW1 user_state, user(vec)
    int maxuser;
    vector<int> user_state; //login table
    vector<string> user;    //login username
    string Split_cmd(string cmd, int uid);
    string Parase(vector<string> cmd_list, int uid);
    string Register(vector<string> cmd_list);
    string Login(vector<string> cmd_list, int uid);
    string Logout(int uid);
    string Whoami(int uid);
    string List_user();
    //Message Box
    vector<Mailbox> user_list;
    string Send(vector<string> cmd_list, int uid);
    string List_msg(int uid);
    string Receive(vector<string> cmd_list, int uid);
    //HW2
    int serial_number = 0;
    int get_sn()
    {
        return ++serial_number;
    }
    int month, day;
    string CreateBoard(vector<string> cmd_list, int uid);
    string CreatePost(vector<string> cmd_list, int uid);
    string ListBoard();
    string ListPost(vector<string> cmd_list);
    string Read(vector<string> cmd_list);
    string DeletePost(vector<string> cmd_list, int uid);
    string UpdatePost(vector<string> cmd_list, int uid);
    string LeaveComment(vector<string> cmd_list, int uid);
};

BBS_server::BBS_server(int max_client)
{
    for (int i = 0; i < max_client; i++)
    {
        user_state.push_back(0);
        user.push_back("");
    }
    maxuser = max_client;
    time_t now = time(0);
    tm *ltm = localtime(&now);
    month = 1 + ltm->tm_mon;
    day = ltm->tm_mday;
    //cout << "Month: " << 1 + ltm->tm_mon << endl;
    //cout << "Day: " << ltm->tm_mday << endl;
}

BBS_server::~BBS_server()
{
    //clean db, mb
    db.clear();
    user_list.clear();
    //clean
    board_list.clear();
    board_index.clear();
    post_list.clear();
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
        //cout << "start split" << endl;
        stringstream ss;
        string out;
        int split = 1, post_c = 0, create = 0, update = 0, comment = 0;
        //cout << "cmd: " << cmd;
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
                //cmd:create-post
                string title, content, gg;
                if (ss.eof())
                {
                    break;
                }
                ss >> out;
                //cout << "f out --t or --con ? :" << out << endl;
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
                    //cout << "title= " << title << endl;
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
                    //cout << "content= " << content << endl;
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
                //cmd: update-post
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
                //cmd: comment
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
        return Logout(uid);
    }
    else if (cmd_list[0] == "whoami")
    {
        return Whoami(uid);
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
        return Send(cmd_list, uid);
    }
    else if (cmd_list[0] == "list-msg")
    {
        return List_msg(uid);
    }
    else if (cmd_list[0] == "receive")
    {
        return Receive(cmd_list, uid);
    }
    else if (cmd_list[0] == "create-board")
    {
        return CreateBoard(cmd_list, uid);
    }
    else if (cmd_list[0] == "create-post")
    {
        //return error;
        return CreatePost(cmd_list, uid);
    }
    else if (cmd_list[0] == "list-board")
    {
        return ListBoard();
    }
    else if (cmd_list[0] == "list-post")
    {
        //return error;
        return ListPost(cmd_list);
    }
    else if (cmd_list[0] == "read")
    {
        //return error;
        return Read(cmd_list);
    }
    else if (cmd_list[0] == "delete-post")
    {
        //return error;
        return DeletePost(cmd_list, uid);
    }
    else if (cmd_list[0] == "update-post")
    {
        //return error;
        return UpdatePost(cmd_list, uid);
    }
    else if (cmd_list[0] == "comment")
    {
        //return error;
        return LeaveComment(cmd_list, uid);
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

string BBS_server::Login(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 3)
    {
        return "Usage: login <username> <password>\n";
    }
    if (user_state[uid] == 1)
    {
        //already login
        return "Please logout first.\n";
    }
    for (int i = 0; i < maxuser; i++)
    {
        if (user_state[i] == 1)
        {
            //other login user
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
        //user exist
        if (cmd_list[2] == iter->second)
        {
            //login successfully
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

string BBS_server::Logout(int uid)
{
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

string BBS_server::Whoami(int uid)
{
    if (user_state[uid] == 0)
    {
        return "Please login first.\n";
    }
    else
    {
        return user[uid] + "\n";
    }
}

string BBS_server::Send(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 3)
    {
        return "Usage: send <username> <message>\n";
    }
    if (user_state[uid] == 0)
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
                it = user_list[i].sender_list.find(user[uid]);
                if (it != user_list[i].sender_list.end())
                {
                    //send before, mail count+1
                    int mail_count = it->second;
                    mail_count++;
                    user_list[i].sender_list[user[uid]] = mail_count;
                }
                else
                {
                    user_list[i].sender_list[user[uid]] = 1;
                }
                //store msg into msg_list
                Message msg(user[uid], cmd_list[2]);
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

string BBS_server::List_msg(int uid)
{
    if (user_state[uid] == 0)
    {
        return "Please login first.\n";
    }
    //find your mailbox
    for (int i = 0; i < user_list.size(); i++)
    {
        if (user_list[i].owner == user[uid])
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

string BBS_server::Receive(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 2)
    {
        return "Usage: receive <username>\n";
    }
    if (user_state[uid] == 0)
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
            if (user_list[i].owner == user[uid])
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

/////////////////////HW2/////////////////////////////////
string BBS_server::CreateBoard(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 2)
    {
        return "Usage: create-board <name>\n";
    }
    if (user_state[uid] == 0)
    {
        return "Please login first.\n";
    }
    string boardname = cmd_list[1];
    Board newboard(boardname, user[uid]);
    pair<map<string, Board>::iterator, bool>
        ret;
    ret = board_list.insert(pair<string, Board>(boardname, newboard));
    if (ret.second == true)
    {
        board_index.push_back(newboard);
        return "Create board successfully.\n";
    }
    else
    {
        return "Board already exists.\n";
    }
}

string BBS_server::ListBoard()
{
    string list;
    list.append("Index Name Moderator\n");
    int index = 1;
    for (auto it = board_index.begin(); it != board_index.end(); it++)
    {
        list.append(to_string(index) + " " + (*it).board_name + " " + (*it).moderater + "\n");
        index++;
    }
    return list;
}

string BBS_server::CreatePost(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 6)
    {
        return "Usage: create-post <board-name> --title <title> --content <content>\n";
    }
    if (((cmd_list[2] == "--title") && (cmd_list[4] == "--content")) || ((cmd_list[2] == "--content") && (cmd_list[4] == "--title")))
    {
        if (user_state[uid] == 0)
        {
            return "Please login first.\n";
        }
        auto it = board_list.find(cmd_list[1]);
        if (it != board_list.end())
        {
            if (cmd_list[2] == "--title")
            {
                string t;
                t.append(to_string(month));
                t.append("/");
                t.append(to_string(day));
                int sn = get_sn();
                Post newpost(sn, user[uid], t, cmd_list[1], cmd_list[3]);
                newpost.CreateContent(cmd_list[5]);
                pair<map<int, Post>::iterator, bool> ret;
                ret = post_list.insert(pair<int, Post>(sn, newpost));
                if (ret.second == true)
                {
                    //board insert post
                    it->second.CreatePost(newpost);
                    return "Create post successfully.\n";
                }
                else
                {
                    return "";
                }
            }
            else
            {
                string t;
                t.append(to_string(month));
                t.append("/");
                t.append(to_string(day));
                int sn = get_sn();
                Post newpost(sn, user[uid], t, cmd_list[1], cmd_list[5]);
                newpost.CreateContent(cmd_list[3]);
                pair<map<int, Post>::iterator, bool> ret;
                ret = post_list.insert(pair<int, Post>(sn, newpost));
                if (ret.second == true)
                {
                    return "Create post successfully.\n";
                }
                else
                {
                    return "";
                }
            }
        }
        else
        {
            return "Board does not exist.\n";
        }
    }
    else
    {
        return "Usage: create-post <board-name> --title <title> --content <content>\n";
    }
}

string BBS_server::ListPost(vector<string> cmd_list)
{
    if (cmd_list.size() != 2)
    {
        return "Usage: list-post <board-name>\n";
    }
    auto it = board_list.find(cmd_list[1]);
    if (it != board_list.end())
    {
        return it->second.ShowBoard();
    }
    else
    {
        return "Board does not exist.\n";
    }
}

string BBS_server::Read(vector<string> cmd_list)
{
    if (cmd_list.size() != 2)
    {
        return "Usage: read <post-S/N>\n";
    }
    auto ret = post_list.find(stoi(cmd_list[1]));
    if (ret != post_list.end())
    {
        return ret->second.ShowPost();
    }
    else
    {
        return "Post does not exist.\n";
    }
}

string BBS_server::DeletePost(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 2)
    {
        return "Usage: delete-post <post-S/N>\n";
    }
    if (user_state[uid] == 0)
    {
        return "Please login first.\n";
    }
    auto ret = post_list.find(stoi(cmd_list[1]));
    if (ret != post_list.end())
    {
        if (ret->second.author != user[uid])
        {
            return "Not the post owner.\n";
        }
        post_list.erase(ret);
        return "Delete successfully.\n";
    }
    else
    {
        return "Post does not exist.\n";
    }
}

string BBS_server::UpdatePost(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 4)
    {
        return "Usage: update-post <post-S/N> --title/content <new>\n";
    }
    if (cmd_list[2] != "--title" && cmd_list[2] != "--content")
    {
        return "Usage: update-post <post-S/N> --title/content <new>\n";
    }
    if (user_state[uid] == 0)
    {
        return "Please login first.\n";
    }
    auto ret = post_list.find(stoi(cmd_list[1]));
    if (ret != post_list.end())
    {
        if (ret->second.author != user[uid])
        {
            return "Not the post owner.\n";
        }
        ret->second.UpdatePost(cmd_list[2], cmd_list[3]);
        return "Update successfully.\n";
    }
    else
    {
        return "Post does not exist.\n";
    }
}

string BBS_server::LeaveComment(vector<string> cmd_list, int uid)
{
    if (cmd_list.size() != 3)
    {
        return "Usage: comment <post-S/N> <comment>\n";
    }
    if (user_state[uid] == 0)
    {
        return "Please login first.\n";
    }
    auto ret = post_list.find(stoi(cmd_list[1]));
    if (ret != post_list.end())
    {
        Comment newcomment(user[uid], cmd_list[2]);
        ret->second.CreateComment(newcomment);
        return "Comment successfully.\n";
    }
    else
    {
        return "Post does not exist.\n";
    }
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
                    //cout << "Adding to list of sockets as " << i << endl;
                    break;
                }
            }
            //greeting
            send(newfd, greeting1, strlen(greeting1), 0);
            send(newfd, greeting2, strlen(greeting2), 0);
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
                //cout << "request from sd: " << sd << endl;
                //receive cmd from client
                char buff[1024];
                memset(buff, 0, 1024);
                auto r = recv(sd, (char *)buff, 1024, 0);
                if (r == 0)
                {
                    //cout << "receive fail" << endl;
                    //close client fd
                    bbs.user_state[i] = 0;
                    bbs.user[i] = "";
                    client_socket[i] = 0;
                    close(sd);
                }
                else
                {
                    string cmd(buff);
                    //parase cmd to reply
                    string reply = bbs.Split_cmd(cmd, i);
                    if (reply == "exit")
                    {
                        if (bbs.user_state[i] == 1)
                        {
                            string bye_msg = "Bye, " + bbs.user[i] + ".\n";
                            send(sd, (char *)bye_msg.c_str(), bye_msg.length() * sizeof(bye_msg[0]), 0);
                        }
                        //close client fd
                        bbs.user_state[i] = 0;
                        bbs.user[i] = "";
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
