#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <boost/filesystem.hpp>

using namespace std;
using namespace boost::filesystem;


//Function for displaying an output
int writebuffer (int fd, const char* buffer , ssize_t size , int* status ){
    ssize_t written_bytes = 0;
    while( written_bytes < size ) {
        ssize_t written_now = write(fd, buffer + written_bytes, size - written_bytes );
        if( written_now == -1){
            if (errno == EINTR)
                continue;
            else{
                *status = errno;
                return -1;
            }
        }else
            written_bytes += written_now;
    }
    return 0;
}

void print_sth(string info){
    int status;
    int out;
    auto *in = new char[info.length() + 1];
    strcpy(in, info.c_str());
    out = writebuffer(STDOUT_FILENO, in, info.length(), &status);
    if (out){
        cerr << "Writebuffer error" << endl;
    }
}

// Reads user answer
string read_confirmation(){
    string line;
    while (true) {
        if (!getline(cin, line)) {
            cerr << "error: unexpected end of file" << endl;
            exit(EXIT_FAILURE);
        }

        transform(line.begin(), line.end(), line.begin(),
                  [](unsigned char x){return tolower(x);});

        if (line == "y" || line == "n" || line == "a" || line == "c") {
            break;
        } else
            cout << "You can enter only Y|y, N|n, A|a, C|c" << endl;
    }
    return line;
}




int main(int argc, char** argv) {

    // Check input
    if (argc == 1){
        cerr << "Try './myrm -h|--help' for more information." << endl;
        return 1;
    }

    // Vectors with commands, names of files
    vector<string> commands;
    vector<string> filenames;

    // Initialize command_sign to find commands
    const char* command_sign = "-";

    // Getting info from User
    for (int i = 1; i < argc; ++i) {
        string tmp(argv[i]);
        if (tmp == "-h" || tmp == "--help") {
            string desc = "\n Usage: ./myrm [-h|--help] [-f] [-R] <file1> <file2> <file3>\n Needs your confirmation to remove the FILE(s). \n -f - if you don't want to be asked \n -R - if you want to remove directories as well.\n\n";
            print_sth(desc);
            return 0;
        } else if (tmp.substr(0,1) == command_sign)
            commands.push_back(tmp);
        else
            filenames.push_back(tmp);
    }


    // Check if user entered filenames.
    if (filenames.empty()){
        cerr << "Please enter at least one filename." << endl;
        return 1;
    }

    // Chek if all commands are right.
    for (string command: commands){
        if (command != "-f" && command != "-R"){
            cerr << "Command " << command << " doesn't exist. Use -h|--help for more information" << endl;
            return 1;
        }
    }


    for (string f: filenames) {
        string file = "./" + f;
        if (find(commands.begin(), commands.end(), "-f") == commands.end()){ //no -f command -> asks for confirmation

            string question = "Do you want to delete file '" + file + "'? (Y[es]/N[o]/A[ll]/C[ancel])\n";
            print_sth(question);

            string line = read_confirmation();
            if (line == "n"){
                continue;
            } else if (line == "c"){
                break;
            } else if (line == "a"){
                commands.push_back("-f");
            }
        }

        // Check if file exists
        if (!boost::filesystem::exists(file)){
            cerr << "Cannot remove '" << file << "': No such file or directory" << endl;
            continue;
        }

        // Check if file is a directory
        file_status s = status(file);
        if (is_directory(s)){
            if (find(commands.begin(), commands.end(), "-R") != commands.end()){
                boost::filesystem::remove_all("./" + file);
                cout << "deleted '" << file << "' (directory)" << endl;
            } else
                cout << "Isn't deleted '" << file << "' (directory)" << endl;
        } else {
            boost::filesystem::remove(file);
            cout << "deleted '" << file << "'"<< endl;
        }
    }
    return 0;
}