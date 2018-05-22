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

int main(int argc, char** argv) {

    int status;

    if (argc < 2) {
        cerr << "You didn't enter any info" << endl;
        return 1;
    } else if(argc == 2){
        string tmpr(argv[1]);
        if (tmpr == "-A"){
            cerr << "You didn't enter any filenames!" << endl;
            return 1;
        }
    }

    vector <string> files;
    int command = 0;
    int outinfo;
    //  ifstream file;

    for (int i = 1; i < argc; ++i) {
        string tmp(argv[i]);
        if (tmp == "-h" || tmp == "--help") {
            string desc = "Use -A to see all the info from files (with hex codes) and then filenames divided by whitespaces.";
            auto *description = new char[desc.length() + 1];
            strcpy(description, desc.c_str());
            outinfo = writebuffer(STDOUT_FILENO, description, desc.length(), &status);
            if (outinfo){
                cerr << "Writebuffer error" << endl;
                return 1;
            }
            return 0;
        }
        else if (tmp == "-A")
            command = 1;
        else{
            files.push_back(tmp);
        }
    }
    for(string filename : files){
        string fileinfo = "";
        char text;

        const char *cstring = filename.c_str();
        int filen = open(cstring, O_RDONLY);
        if (filen == -1){
            if (errno == EINTR){
                continue;
            } else{
                cerr << errno << endl;
                return 2;
            }
        }

        ssize_t readinfo;
        off_t nbytes = lseek(filen,0,SEEK_END);
        if(nbytes == -1){
            cerr << "Failed to get bytes" << endl;
        }
        int errc;
        errc = lseek(filen, 0, SEEK_SET);
        if(errc == -1){
            cerr << "Failed to get bytes" << endl;
        }
        char tempbuf[nbytes];
        int counter = 0;

        while ((readinfo = read(filen, tempbuf, nbytes)) != 0){
            for (int i =0; i < nbytes; ++i){
                if(command){
                    if(isprint(tempbuf[i])){
                        fileinfo += tempbuf[i];
                    }else{
                        counter+=4;
                        char buf[6];
                        sprintf(buf, "\\x%02X", tempbuf[i]);
                        for(int k = 0; k<4; ++k){
                            fileinfo += buf[k];
                        }
                    }
                }else{
                    fileinfo += tempbuf[i];
                }
            }
            print_sth(fileinfo);
        }
        print_sth("\n");
    }
    return 0;
}
