#include <stdio.h>
#include <string.h>
#include <vector>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <algorithm>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <fstream>

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;


bool check_line(string line, string pattern, bool ignore_case, bool invert_match, bool regexp_specified, bool file){

    bool found = false;

    if (ignore_case) {
        boost::algorithm::to_lower(line);
        boost::algorithm::to_lower(pattern);
    }
    if (!invert_match) {
        if (!regexp_specified){
            if (line.find(pattern) != string::npos){
                found = true;
                if (!file) {
                    cout << "found: ";
                }
                cout << line << endl;
            }
        } else {
            boost::regex reg_ex(pattern);
            boost::smatch results;
            if (boost::regex_match(line.c_str(), results, reg_ex)){
                found = true;
                if (!file) {
                    cout << "found: ";
                }
                cout << line << endl;
            }
        }
    } else {
        if (!regexp_specified){
            if (line.find(pattern) == string::npos){
                found = true;
                if (!file) {
                    cout << "found: ";
                }
                cout << line << endl;
            }
        } else {
            cout << "shos z regexp" << endl;
        }
    }
    return found;
}


int main(int argc, char** argv) {

    // init
   // bool recursively_mode = false;
    bool invert_match = false;
    bool ignore_case = false;
    bool filename_specified = false;
    bool regexp_specified = false;

    vector<string> paths;

    // parse command line args
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h",
             "grep [-h|--help] [-v|--invert-match] [-i|--ignore-case] [--file=<filename>] [--regexp=’<regexpression>’ | string]")
            ("invert-match,v", "select non-matching lines")
            ("ignore-case,i", "ignore case distinctions")
            ("file", po::value<std::string>(), "specify a filename")
            ("regexp", po::value<std::string>(), "use pattern for matching");
    po::variables_map vm;

    if (argc == 1) {
        cerr << "You did't enter any info. Use './mygrep -h|--help' for more info." << endl;
        return EXIT_FAILURE;
    }

    string pattern;
    string filename;

    try {
        auto parsed = po::command_line_parser(argc, argv).options(desc).run();
        paths = po::collect_unrecognized(parsed.options, po::include_positional);

        po::store(parsed, vm);
        po::notify(vm);

        // Print help and version of the program
        if (vm.count("help")) {
            cout << desc << endl;
            return EXIT_SUCCESS;
        }

        if (vm.count("invert-match")) {
            invert_match = true;
        }

        if (vm.count("ignore-case")) {
            ignore_case = true;
        }

        if (vm.count("file")) {
            filename_specified = true;
            filename = vm["file"].as <string>();
        }

        if (vm.count("regexp")) {
            regexp_specified = true;
            pattern = vm["regexp"].as <string>();
        }

    } catch (po::error &e) {
        cerr << "ERROR: " << e.what() << endl;
        cerr << desc << endl;
        return EXIT_FAILURE;
    }

    bool repeat = false;
    const char* command_sign = "-";
    for(int i = 1; i < argc; ++i) {
        string tmp(argv[i]);
        if (tmp.substr(0,1) != command_sign) {
            if (regexp_specified) {
                cerr << "Please, enter either regexp or string!" << endl;
                return EXIT_FAILURE;
            }

            if (repeat) {
                cerr << "Please, enter only one string to compare!" << endl;
                return EXIT_FAILURE;
            }
            pattern = tmp;
            repeat = true;
        }
    }

    bool found = false;
    if (filename_specified) {
        ifstream input(filename);
        if (!input.good()){
            cerr << "Incorrect filename!" << endl;
            return EXIT_FAILURE;
        }
        for(string line; getline(input,line);)
        {
            found = check_line(line, pattern, ignore_case, invert_match, regexp_specified, filename_specified);
        }

    } else {
        for (string line; getline(cin, line);) {
            found = check_line(line, pattern, ignore_case, invert_match, regexp_specified, filename_specified);
        }
    }

    if (!found){
        cout << "Nothing found :(" << endl;
    }

    return 0;
}
