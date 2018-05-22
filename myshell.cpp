#include <stdio.h>
#include <unistd.h>
#include<iostream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sstream>
#include <iterator>
#include <string.h>
#include <wordexp.h>
#include <functional>

#define GetCurrentDir getcwd

using std::cout;
using std::cerr;
using std::endl;
using std::string;
extern char **environ;

using namespace std;

string GetCurrentWorkingDir(void) {

    char buff[FILENAME_MAX];

    GetCurrentDir(buff, FILENAME_MAX);
    string current_working_dir(buff);

    return current_working_dir;
}

//merrno function
int merrno(string command, string line, int merno) {

    if (!command.compare(line)) {

        std::cout << merno << std::endl;

        return merno;
    }
    else if (!line.substr(command.length() + 1, line.find(' ')).compare("-h") ||
               !line.substr(command.length() + 1, line.find(' ')).compare("--help")) {

        cout << "Type 'merrno' to get an exit code of last executed program" << endl;
        merno = 0;
        std::cout << merno << std::endl;

        return merno;
    }
    else {

        cout << "Bad parameters. Try again" << endl;
        merno = 1;
        std::cout << merno << std::endl;

        return merno;
    }
}

//mpwd function
int mpwd(string command, string line, int merno, string curdir) {

    if (!command.compare(line)) {

        cout << curdir << endl;

        return merno;
    }
    else if (!line.substr(command.length() + 1, line.find(' ')).compare("-h") ||
               !line.substr(command.length() + 1, line.find(' ')).compare("--help")) {

        cout << "Type 'mpwd' to get a path of current directory" << endl;
        merno = 0;

        return merno;
    }
    else {

        cout << "Bad parameters. Try again" << endl;
        merno = 1;

        return merno;
    }
}

//mcd function
string mcd(string curr_dir, string command, string line) {

    string new_dir = line.substr(line.find(" ") + 1);
    struct stat sb;
    const char *cstr = new_dir.c_str();

    if (!command.compare(line)) {

        cout << "Bad parameters. Try again" << endl;

        return "bad";
    }

    if (!new_dir.compare("-h") || !new_dir.compare("--help")) {

        cout << "Type 'mcd' and a path to change current directory" << endl;

        return "good";

    }
    else if (!new_dir.compare(".")) {

        return curr_dir;
    }
    else if (!new_dir.compare("..")) {

        curr_dir = curr_dir.substr(0, curr_dir.find_last_of("/\\"));

        return curr_dir;
    }
    else if (stat(cstr, &sb) == 0 && S_ISDIR(sb.st_mode)) {

        return new_dir;
    }
    else {

        cout << "Bad parameters. Try again" << endl;

        return "bad";
    }
}

bool is_number(const std::string &s) {

    return !s.empty() && find_if(s.begin(), s.end(), [](char c) { return !isdigit(c); }) == s.end();
}

//mexit function
int mexit(string command, string line, int &check) {

    if (!command.compare(line)) {

        std::cout << "Process finished with exit code 0" << std::endl;

        return 0;
    }

    string value = line.substr(line.find(" ") + 1);

    if (!value.compare("-h") || !value.compare("--help")) {

        std::cout << "Type 'mexit' and an exit code to close myshell." << std::endl;

        return 3;
    }
    else if (is_number(value)) {

        std::cout << "Process finished with exit code " << std::stoi(value) << std::endl;

        return std::stoi(value);
    }
    else {

        std::cout << "Bad parameters. Try again" << std::endl;
        check = 0;

        return 2;
    }
}

inline bool file_exists(const std::string &name) {

    if (FILE *file = fopen(name.c_str(), "r")) {

        fclose(file);

        return true;
    }
    else {

        return false;
    }
}

void fork_ex(string command, string line) {

    wordexp_t p;
    char **w;
    char *ch = new char[line.length() + 1];
    strcpy(ch, line.c_str());
    wordexp(ch, &p, 0);
    w = p.we_wordv;

    pid_t parent = getpid();
    pid_t pid = fork();

    if (pid == -1) {
        std::cout << "Bad parameters. Try again" << std::endl;
        exit(EXIT_FAILURE);
    }
    else if (pid > 0) {

        int status;
        waitpid(pid, &status, 0);
    }
    else {
        // We are the child
        execve(command.c_str(), w, environ);
        exit(EXIT_FAILURE);   // exec never returns
    }
}


int executeSriptFile(stringstream &fileName){
//
    ifstream inFile("../"+fileName.str());
//
    if(!inFile){
//
        cout << "there are no such file" << endl;
        return -1;
    }
//
    string line;
    while (std::getline(inFile, line))
    {
        istringstream iss(line);
        string command;
        if (!(iss >> command)) { break; }
        if(command.c_str()[0] != '#'){
//
            fork_ex(command.c_str(),line);
        }
    }

    inFile.close();
//
    return 1;
}


int main(int argc, char** argv) {
//
    if(argc == 2){
//
//      cout << "You write more than 1 parameter" << endl;
        string nameFile = argv[1];
        stringstream stream{nameFile};
//
        return executeSriptFile(stream);
    }
    else if(argc > 2){
//
        cout << "Incorect parameters" << endl;
        return -1;
    }

    string line;
    string command;
    int merno = 0;
    string currentdir = GetCurrentWorkingDir();



    while (true) {

        cout << currentdir << " $ ";
        getline(cin, line);

        if (line.empty())

            continue;

        if (!line.compare("-h") || !line.compare("--help")) {

            cout << "You can call this commands: ... ... " << endl;

            continue;
        }

        if(line[0] == '.'){
            command = line.substr(1, line.find(' '));
            stringstream stringstream1{command};
            executeSriptFile(stringstream1);
            continue;
        }

        if (line[0] != '\"') {

            command = line.substr(0, line.find(' '));
        }
        else {

            command = line.substr(1, line.find_last_of('\"') - 1);
        }
        if (!command.compare("merrno")) {

            merno = merrno(command, line, merno);
        }
        else if (!command.compare("mpwd")) {

            merno = mpwd(command, line, merno, currentdir);
        }
        else if (!command.compare("mcd")) {

            string mcdreturn = mcd(currentdir, command, line);

            if (!mcdreturn.compare("bad")) {

                merno = 1;
            }
            else {

                merno = 0;
            }
            if (mcdreturn.compare("bad") && mcdreturn.compare("good"))

                currentdir = mcdreturn;
        }
        else if (!command.compare("mexit")) {

            int check = 1;
            merno = mexit(command, line, std::ref(check));

            if (check) {

                break;
            }
        }
        else {

            if (file_exists(command)) {

                fork_ex(command, line);
            }
            else {

                cout << "Enter the command as the first argument!" << endl;
                cout << "Use -h | --help to see available commands" << endl;
                merno = 1;
            }
        }
    }

    return merno;
}
