#include <iostream>
#include <vector>
#include <cstring>
#include <fcntl.h>
#include <unistd.h>

using namespace std;


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

void read_file(string file, bool mode){

    const char *cstring = file.c_str();
    int filen = open(cstring, O_RDONLY);

    if (filen < 0){
        if (errno != EINTR){
            cerr << errno << endl;
            exit(2);
        }
    }

    off_t nbytes = lseek(filen,0,SEEK_END);
    __off_t errc = lseek(filen, 0, SEEK_SET);

    if(nbytes == -1 || errc == -1){
        cerr << "Failed to get bytes" << endl;
    }

    char tmp_buf[nbytes];
    int counter = 0;
    string content;
    ssize_t readinfo;

    while ((readinfo = read(filen, tmp_buf, nbytes)) != 0){
        for (int i =0; i < nbytes; ++i){
            if(mode){
                if(isprint(tmp_buf[i])){
                    content += tmp_buf[i];
                }else{
                    counter+=4;
                    char buf[6];
                    sprintf(buf, "\\x%02X", tmp_buf[i]);
                    for(int k = 0; k<4; ++k){
                        content += buf[k];
                    }
                }
            }else{
                content += tmp_buf[i];
            }
        }
        print_sth(content);
    }
    print_sth("\n");
}

int main(int argc, char** argv) {

    int status;


    if (argc <= 1) {
        cerr << "Try -h or --help" << endl;
        return 1;
    } else if(argc == 2){
        string tmpr(argv[1]);
        if (tmpr == "-A"){
            cerr << "Enter a file name or names !" << endl;
            return 1;
        }
    }

    vector <string> files;
    bool A_mode = false;
    int outinfo;

    for (int i = 1; i < argc; ++i) {
        string tmp(argv[i]);
        if (tmp == "-h" || tmp == "--help") {
            string desc = "Use -A to see all the info from files (with hex codes) and then filenames divided by whitespaces.";

            outinfo = writebuffer(STDOUT_FILENO, desc.c_str(), desc.length(), &status);
            if (outinfo){
                cerr << "Writebuffer error" << endl;
                return 1;
            }
            return 0;
        }
        else if (tmp == "-A")
            A_mode = true;
        else{
            read_file(tmp, A_mode);
        }
    }
    return 0;
}
