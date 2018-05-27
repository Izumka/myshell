#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iterator>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include <map>
#include <fstream>

#define GetCurrentDir getcwd

using namespace std;

//Functions-Getters

string get_cr_date(const char *name){
    struct stat t_stat;
    char buffer[80];
    stat(name, &t_stat);
    struct tm * timeinfo = localtime(&t_stat.st_ctime); // or gmtime() depending on what you want
    strftime(buffer,sizeof(buffer),"%d-%m-%Y %I:%M:%S",timeinfo);
    string str(buffer);
    return str;
}

string GetCurrentWorkingDir( void ) {
    char buff[FILENAME_MAX];
    GetCurrentDir( buff, FILENAME_MAX );
    string current_working_dir(buff);
    return current_working_dir;
}

ifstream::pos_type getSize(string str) {
    const char * filename = str.c_str();
    ifstream in(filename, ifstream::ate | ifstream::binary);
    return in.tellg();
}

string getFileExtension(const string& filename)
{
    if(filename.find_last_of(".") != string::npos)
        return filename.substr(filename.find_last_of(".")+1);
    return "";
}

//Functions for verification
int is_just_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

bool is_dir(const char* path) {
    struct stat buf;
    stat(path, &buf);
    return S_ISDIR(buf.st_mode);
}

inline bool file_exists (const std::string& name) {
    if (FILE *file = fopen(name.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

vector<string> checkSpecialFiles(vector<string> filenames){
    vector<string> new_files;
    struct stat path_stat;
    for (auto n: filenames){
        const char * name = n.c_str();
        stat(name, &path_stat);

        if (stat(name, &path_stat) == 0 && path_stat.st_mode & S_IXUSR){  //executable
            new_files.push_back("*" + n);
        }
        else if (S_ISLNK(path_stat.st_mode) == 0){ //symlink
            new_files.push_back("@" + n);
        }
        else if (S_ISFIFO(path_stat.st_mode) == 0){ //named pipe
            new_files.push_back("|" + n);
        }
        else if (S_ISSOCK(path_stat.st_mode) == 0){ //socket
            new_files.push_back("=" + n);
        }
        else if (S_ISREG(path_stat.st_mode)!=0 && S_ISDIR(path_stat.st_mode)!=0){ //other special files
            new_files.push_back("?" + n);
        }
    }
    return new_files;
}

bool check_special(string n){
    struct stat path_stat;
    const char * name = n.c_str();
    stat(name, &path_stat);
    if((stat(name, &path_stat) == 0 && path_stat.st_mode & S_IXUSR) || S_ISLNK(path_stat.st_mode) == 0 || S_ISFIFO(path_stat.st_mode) == 0 || S_ISSOCK(path_stat.st_mode) == 0 || S_ISREG(path_stat.st_mode)!=0 && S_ISDIR(path_stat.st_mode)!=0){
        return 1;
    }
    return 0;
}

//Sorting Functions
vector<string> u_func(vector<string> files){
    return files;
}

vector<string> sortByName(vector<string> files){
    sort(begin(files), end(files), [](string const &a, string const &b)
    {
        return lexicographical_compare(begin(a), end(a), begin(b), end(b), [](string::value_type a, string::value_type b)
        {
            return tolower(a) < tolower(b); //case-insensitive
        });
    });
    return files;
}

vector<string> sortBySize(vector<string> files){
    vector<pair<string,int>> files_size;
    for (int i = 0; i < files.size(); i++) {
        files_size.push_back(make_pair(files[i], getSize(files[i])));
    }
    sort(files_size.begin(), files_size.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    for (int i = 0; i < files.size(); i++) {
        files[i] = files_size[i].first;
    }
    return files;
}

vector<string> sortByTime(vector<string> files){
    vector<pair<string,string>> files_dates;
    for (int i = 0; i < files.size(); i++) {
        files_dates.push_back(make_pair(files[i], get_cr_date(files[i].c_str())));
    }
    sort(files_dates.begin(), files_dates.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    for (int i = 0; i < files.size(); i++) {
        files[i] = files_dates[i].first;
    }
    return files;
}

vector<string> sortByExtention(vector<string> files){
    vector<pair<string,string>> files_extentions;
    for (int i = 0; i < files.size(); i++) {
        files_extentions.push_back(make_pair(files[i], getFileExtension(files[i])));
    }
    sort(files_extentions.begin(), files_extentions.end(), [](auto &left, auto &right) {
        return left.second < right.second;
    });
    for (int i = 0; i < files.size(); i++) {
        files[i] = files_extentions[i].first;
    }
    return files;
}

vector<string> sort_by_dir(vector<string>files,string path){
    vector<string>new_files;
    int check = 0;
    while(new_files.size()!=files.size()){
        for(string file: files){
            if(check){
                if(!is_dir((path + "/" + file).c_str())){
                    new_files.push_back(file);
                }
            }else{
                if(is_dir((path + "/" + file).c_str())){
                    new_files.push_back(file);
                }
            }
        }
        check++;
    }
    return new_files;
}

vector<string> sort_by_special(vector<string>files){
    vector<string>new_files;
    int check = 0;
    while(new_files.size()!=files.size()){
        for(string file: files){
            if(check){
                if(!check_special(file)){
                    new_files.push_back(file);
                }
            }else{
                if(check_special(file)){
                    new_files.push_back(file);
                }
            }
        }
        check++;
    }
    return new_files;
}

//Function related to '-r' parameter
vector<string> reverseOrder(vector<string> filenames){
    reverse(filenames.begin(), filenames.end());
    return filenames;
}

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

//Functions that adds '/' to folders
vector<string> add_sign(vector<string> files, string path){
    vector<string>new_files;
    for(string file: files){
        string s = file.substr(0, file.find(' '));
        s = path + "/" + s;
        char * ch = new char[s.length() + 1];
        strcpy(ch,s.c_str());
        if(is_dir(ch)){
            new_files.push_back("/" + file);
        }else{
            new_files.push_back(file);
        }
    }
    return new_files;
}

//Transfers files from path
vector<string> files_from_path(string path, int f){
    struct stat sb;
    const char *cstr;
    vector<string> files;
    cstr = path.c_str();
    if(stat(cstr, &sb) == 0 && S_ISDIR(sb.st_mode)){
        char * ch = new char[path.length() + 1];
        strcpy(ch,path.c_str());
        DIR *dir;
        struct dirent *ent;

        dir = opendir (ch);
        if(f){
            while ((ent = readdir (dir)) != NULL) {
                if(string(ent->d_name) != "." && string(ent->d_name) != ".."){
                    if(is_dir(string(path + "/" + ent->d_name).c_str())){
                        files.push_back(string(ent->d_name));
                    }
                }
            }
        }else{
            while ((ent = readdir (dir)) != NULL) {
                if(string(ent->d_name) != "." && string(ent->d_name) != ".."){
                    files.push_back(string(ent->d_name));
                }
            }
        }
        closedir (dir);
    }
    return files;

}
//Function related to '-l' parameter
vector<string> l_function(vector<string> files){
    vector<string> new_files;
    for(string i: files){
        new_files.push_back(i + "   " + to_string(getSize(i)) + "   " + get_cr_date(i.c_str()));
    }
    return new_files;
}

//Function that sorts set of files by given sorting options
vector<string> sorting_func(vector<string> files, string sorting_options, string path){
    int count = 0;
    map <char,vector<string>> sort_opt = {{'t', sortByTime(files)},{'U', u_func(files)},{'S', sortBySize(files)},{'X', sortByExtention(files)},{'N', sortByName(files)}};;
    map <char,vector<string>> other_sort = {{'D', sort_by_dir(files,path)}, {'s',sort_by_special(files)}};;
    if(sorting_options.empty()){
        return sort_opt['N'];
    }else{
        for(char &let: sorting_options){
            if(!(sort_opt.find(let) == sort_opt.end()) || !(other_sort.find(let) == other_sort.end())){
                if(count){
                    if(sort_opt.find(let) == sort_opt.end() && !(other_sort.find(let) == other_sort.end())){
                        files = other_sort[let];
                    }else{
                        cerr<<"Bad Info"<<endl;
                    }
                } else if(!(sort_opt.find(let) == sort_opt.end())){
                    count++;
                    files = sort_opt[let];
                }
                sort_opt = {{'t', sortByTime(files)},{'U', u_func(files)},{'S', sortBySize(files)},{'X', sortByExtention(files)},{'N', sortByName(files)}};
                other_sort = {{'D', sort_by_dir(files,path)}, {'s',sort_by_special(files)}};
            }else{
                cerr<<"No such parameter: "<<let<<endl;
                print_sth("\n");
            }
        }
        return files;
    }
}

//Function that filters and iterates through given files
vector<string> map_filter_iterator(map <string,int> commands,vector<string> files, string sorting_options,string path){
    map<string, int>::iterator it;
    map <string,vector<string>> filtered = {{"-r", reverseOrder(files)},{"-l", l_function(files)},{"--sort", sorting_func(files,sorting_options,path)}};

    for ( it = commands.begin(); it != commands.end(); it++ )
    {
        if(it->second){
            if(string(it->first)!="-R"){
                files = filtered[it->first];
                filtered = {{"-r", reverseOrder(files)},{"-l", l_function(files)},{"--sort", sorting_func(files,sorting_options,path)}};
            }
        }
    }
    return files;
}

//Function that recursively goes through all subdirectories
void recursive_fold(string path,vector<string> &paths){
    vector<string> folders = files_from_path(path,1);
    if(!(folders.empty())){
        for(string folder: folders){
            if(folder!=".idea" && folder!=""){
                paths.push_back(path+ "/" + folder);
                recursive_fold(path+ "/" + folder, paths);
            }
        }
    }
}

int main(int argc, char** argv) {
    // Initial variables
    string sorting_options;
    struct stat sb;
    const char *cstr;
    string currentdir = GetCurrentWorkingDir();

    // Map of commands
    map <string,int> commands = {{"--sort", 0},{"-r", 0},{"-F", 0},{"-R", 0}, {"-l", 0}};

    // Info sets
    vector <string> files;
    vector <string> paths;
    string com;
    int block = 1;
    //Getting Info from User
    for (int i = 1; i < argc; ++i) {
        string tmp(argv[i]);
        cstr = tmp.c_str();

        com = tmp.substr(0, tmp.find('='));
        if (tmp == "-h" || tmp == "--help") {
            string desc = "\nType a directory path to see all files in it or a single file name to see its name. Use -l to see more info about files, --sort to sort the output. More about ls options in our website: https://cms.ucu.edu.ua/mod/assign/view.php?id=37078 \n";
            print_sth(desc);
            print_sth("\n");
            return 0;
        }else if(tmp == "--"){
            block = 0;
        }
        else if (stat(cstr, &sb) == 0 && S_ISDIR(sb.st_mode)){
            currentdir = tmp;
            paths.push_back(currentdir);
        }
        else if(file_exists(tmp)){
            files.push_back(tmp);
        } else if(block){
            if(!(commands.find(tmp) == commands.end()) || com=="--sort"){
                if(com == "--sort"){
                    commands[com] = 1;
                    string la = tmp.substr(tmp.find('=')+1);
                    if(la != "--sort"){
                        sorting_options = la;
                    }
                }else{
                    commands[tmp] = 1;
                }
            }
        }
        else{
            cerr<< "No such file: "<<tmp<<endl;
        }
    }
    if(paths.empty() && files.empty()){
        paths.push_back(currentdir);
    }
    int empty = 0;
    if(files.empty()){
        empty = 1;
    }

    if(commands["-R"]){
        for(string path: paths){
            recursive_fold(path,paths);
        }
    }
    //Processing Info
    vector<string> fin_files = files;
    vector<string> prost;
    if(!empty){
        prost = map_filter_iterator(commands,files,sorting_options,currentdir);
    }
    if(!(prost.empty())){
        fin_files = prost;
    }

    //Printing Info
    if(!(paths.empty())){
        for(string path: paths){
            files = files_from_path(path, 0);
            vector<string> var = map_filter_iterator(commands, files,sorting_options,path);
            if(!(var.empty())){
                files = var;
            }
            print_sth("\n");
            for(string file: add_sign(files,path)){
                print_sth(file+"\n");
            }
        }
    }

    if(!(files.empty())){
        print_sth("\n");
        for(string file: fin_files){
            print_sth(file+"\n");
        }
    }
}