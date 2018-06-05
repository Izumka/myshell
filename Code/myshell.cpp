#include <stdio.h>
#include <unistd.h>
#include<iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sstream>
#include <iterator>
#include <string.h>
#include <wordexp.h>
#include <functional>
#include <map>
#include <pwd.h>
#include <vector>

#define GetCurrentDir getcwd

using std::cout;
using std::cerr;
using std::endl;
using std::string;

using namespace std;

map<string, string> global_variable;


string GetCurrentWorkingDir(void)
{
    char buff[FILENAME_MAX];

    GetCurrentDir(buff, FILENAME_MAX);
    string current_working_dir(buff);

    return current_working_dir;
}

//merrno function
void merrno(string command, string line, int &ERRNO)
{
    if (!command.compare(line))
    {
        std::cout << ERRNO << std::endl;

        return;
    }
    else if (!line.substr(command.length() + 1, line.find(' ')).compare("-h") ||
               !line.substr(command.length() + 1, line.find(' ')).compare("--help"))
    {
        cout << "Type 'merrno' to get an exit code of last executed program" << endl;
        ERRNO = 0;
        std::cout << ERRNO << std::endl;

        return;
    }
    else
        {
        cout << "Bad parameters. Try again" << endl;
        ERRNO = 1;
        std::cout << ERRNO << std::endl;

        return;
    }
}

//mpwd function
void mpwd(string command, string line, string curdir, int &ERRNO)
{
    if (!command.compare(line))
    {
        cout << curdir << endl;

        return;
    }
    else if (!line.substr(command.length() + 1, line.find(' ')).compare("-h") ||
               !line.substr(command.length() + 1, line.find(' ')).compare("--help"))
    {
        cout << "Type 'mpwd' to get a path of current directory" << endl;
        ERRNO = 0;

        return;
    }
    else
        {
        cout << "Bad parameters. Try again" << endl;
        ERRNO = 1;

        return;
    }
}

//mcd function
string mcd(string curr_dir, string command, string line)
{
    string new_dir = line.substr(line.find(" ") + 1);
    struct stat sb;
    const char *cstr = new_dir.c_str();

    if (!command.compare(line))
    {
        cout << "Bad parameters. Try again" << endl;

        return "-1";
    }
    if (!new_dir.compare("-h") || !new_dir.compare("--help"))
    {
        cout << "Type 'mcd' and a path to change current directory" << endl;

        return "0";
    }
    else if (!new_dir.compare("."))
    {
        return curr_dir;
    }
    else if (!new_dir.compare(".."))
    {
        curr_dir = curr_dir.substr(0, curr_dir.find_last_of("/\\"));

        return curr_dir;
    }
    else if (stat(cstr, &sb) == 0 && S_ISDIR(sb.st_mode))
    {
        return new_dir;
    }
    else
        {
        cout << "Bad parameters. Try again" << endl;

        return "-1";
    }
}

bool is_number(const std::string &s)
{
    return !s.empty() && find_if(s.begin(), s.end(), [](char c) { return !isdigit(c); }) == s.end();
}

//mexit function
void mexit(string command, string line, int &check, int &ERRNO)
{
    if (!command.compare(line))
    {
        std::cout << "Process finished with exit code 0" << std::endl;

        ERRNO = 0;

        return;
    }
    string value = line.substr(line.find(" ") + 1);

    if (!value.compare("-h") || !value.compare("--help"))
    {
        std::cout << "Type 'mexit' and an exit code to close myshell." << std::endl;

        ERRNO = 3;

        return;
    }
    else if (is_number(value))
    {
        std::cout << "Process finished with exit code " << std::stoi(value) << std::endl;

        ERRNO = std::stoi(value);
        return;
    }
    else
        {
        std::cout << "Bad parameters. Try again" << std::endl;
        check = 0;

        ERRNO = 2;
        return;
    }
}

inline bool file_exists(const std::string &name)
{

    if (FILE *file = fopen(name.c_str(), "r"))
    {
        
        fclose(file);

        return true;
    }
    else
        {
        return false;
    }
}

bool exist_in_parrent_dir(string name, string p_dir){

    if (FILE *file = fopen((p_dir+"/"+name).c_str(), "r"))
    {
        fclose(file);

        return true;
    }
    else
    {
        return false;
    };
    
}

void fork_ex(string command, string line, int &ERRNO)
{
    wordexp_t p;
    char **w;
    char *ch = new char[line.length() + 1];
    strcpy(ch, line.c_str());
    wordexp(ch, &p, 0);
    w = p.we_wordv;

    pid_t parent = getpid();
    pid_t pid = fork();

    if (pid == -1)
    {
        std::cout << "Bad parameters. Try again" << std::endl;
        ERRNO = 1;
        exit(EXIT_FAILURE);
    }
    else if (pid > 0)
    {
        int status;
        waitpid(pid, &status, 0);
    }
    else {
        if(file_exists(command)){
            execve(command.c_str(), w, environ);
        } else {
            vector<const char *> args{line.c_str()};
            args.push_back(nullptr);
            execvp(command.c_str(), const_cast<char *const *>(args.data()));
        }
        cout << "Enter the command as the first argument!" << endl;
        cout << "Use -h | --help to see available commands" << endl;

        ERRNO = 1;
        exit(EXIT_FAILURE);   // exec never returns
    }
}


void executeSriptFile(stringstream &fileName, int &ERRNO)
{
    ifstream inFile(fileName.str());

    if(!inFile)
    {
        cout << "there are no such file" << endl;
       ERRNO = - 1;

        return;
    }

    string line;

    while (std::getline(inFile, line))
    {
        istringstream iss(line);
        string command;

        if (!(iss >> command))
        {
            break;
        }
        if(command.c_str()[0] != '#')
        {
            std::cout << command << std::endl;
            fork_ex(command.c_str(),line, ERRNO);
        }
    }

    inFile.close();

    ERRNO = 1;
    return;
}

void var_support(string command)
{
    string key;
    string value;

    key = command.substr(0, command.find('='));
    value = command.substr(command.find('=')+1,command.find(' '));

    if(global_variable.count(key) ==  1)
    {
        global_variable[key] = value;
    }

    global_variable.insert(pair<string,string>(key, value));
}

void mexport(string line, int &ERRNO)
{
    string key;
    string value;
    string command;

    if(line.find('=') != line.find('`'))
    {

        command = line.substr(0, line.size());
        var_support(command);
        key = command.substr(0,command.find('='));

    }
    else
        {
            key = line.substr(0, line.size());
        }
    if(global_variable.count(key) ==  1)
    {
        value = global_variable[key];
        setenv(key.c_str(),value.c_str(), true);
        ERRNO = 0;
        return;
    }
    ERRNO = -1;
    return;
}

string ampersant(string line)
{
    if(global_variable.count(line) != 0)
    {
        return global_variable[line];
    }
    return "";
}

void mecho(string line, int &ERRNO)
{
    string output;

    if(line.find('$') != line.find('`'))
    {
        output = ampersant(line.substr(line.find('$') + 1, line.size()));
    }
    else
    {
        output = line;
    }
    cout << output << endl;
    ERRNO = 0;
    return;
}


int main(int argc, char** argv)
{
    char bufer[FILENAME_MAX];

    int ERRNO = 0;

    string path_to_subprograms = bufer;
    if (const char *path = getenv("PATH")) {
        string to_path = path;
        to_path = path_to_subprograms + ":" + to_path;
        setenv("PATH", to_path.c_str(), 1);
    }

    if(argc == 2)
    {
        string nameFile = argv[1];
        stringstream stream{nameFile};

        executeSriptFile(stream, ERRNO);
    }

    else if(argc > 2)
    {
        cout << "Incorect parameters" << endl;

        return -1;
    }
    string line;
    string command;
    string currentdir = GetCurrentWorkingDir();
    string p_dir = currentdir;


    while (true)
    {
        cout << currentdir << " $ ";
        getline(cin, line);

        if (line.empty())
        {
            continue;
        }
        if (!line.compare("-h") || !line.compare("--help"))
        {
            cout << "You can call this commands: ... ... " << endl;

            continue;
        }
        if(line[0] == '.')
        {
            command = line.substr(1, line.find(' '));
            stringstream stringstream1{command};
            executeSriptFile(stringstream1, ERRNO);

            continue;
        }
        if (line[0] != '\"')
        {
            command = line.substr(0, line.find(' '));
        }
        else
        {
            command = line.substr(1, line.find_last_of('\"') - 1);
        }
        if (!command.compare("merrno"))
        {
            merrno(command, line, ERRNO);
        }
        else if (!command.compare("mecho"))
        {
            mecho(line.substr(line.find(' ') + 1, line.size()), ERRNO);
        }
        else if (!command.compare("mpwd"))
        {
            mpwd(command, line, currentdir, ERRNO);
        }
        else if (!command.compare("mexport"))
        {
            mexport(line.substr(line.find(' ') + 1, line.size()), ERRNO);

        }
        else if (!command.compare("mcd"))
        {
            string mcdreturn = mcd(currentdir, command, line);

            if (mcdreturn.size() > 1)
            {
                chdir(mcdreturn.c_str());
                currentdir = mcdreturn;
            }
            else
            {
                ERRNO = atoi(mcdreturn.c_str());
            }
        }
        else if (!command.compare("mexit"))
        {
            int check = 1;
            mexit(command, line, check, ERRNO);

            if (check)
            {
                break;
            }
        }
        else
            {
            if(line.find('=') != line.find('`'))
            {
                var_support(line);

                continue;
                }
            if (file_exists(command))
            {
                if(line.find("$") != line.find('`'))
                {
                    if(file_exists(ampersant(line.substr(line.find('$') + 1, line.size()))))
                    {
                        line.replace(line.find('$'), ampersant(line.substr(line.find('$') + 1, line.size())).size() , ampersant(line.substr(line.find('$')+ 1, line.size())));
                        fork_ex(command, line, ERRNO);
                    } else {
                        std::cout << "bad param for command" << std::endl;

                        ERRNO = -2;
                    }
                } else {fork_ex(command, line, ERRNO);}
            }
            else if(exist_in_parrent_dir(command,p_dir))
            {
                fork_ex(p_dir+"/"+command, line, ERRNO);
            }

            else
                {
                fork_ex(command, line, ERRNO);
            }
        }
    }
    return ERRNO;
}
