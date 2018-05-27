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
#include <regex>

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;


int check_line(string& line, string& pattern, bool ignore_case, bool invert_match, bool regexp_specified, bool file){


    if (ignore_case) {
        boost::algorithm::to_lower(line);
        boost::algorithm::to_lower(pattern);
    }
    if (!invert_match) {
        if (!regexp_specified){
            if (line.find(pattern) != string::npos){
                if (!file)
                    cout << "found: ";
                cout << line << endl;
                return 1;
            }
        } else {
          regex e (pattern);
          if (regex_search(line, e)){
              if (!file)
                  cout << "found: ";
              cout << line << endl;
              return 1;
          }
        }
    } else {
        if (!regexp_specified){
            if (line.find(pattern) == string::npos){
                if (!file)
                    cout << "found: ";
                cout << line << endl;
                return 1;
            }
        } else {
            regex e (pattern);
            if (!regex_search(line, e)){
                if (!file)
                    cout << "found: ";
                cout << line << endl;
                return 1;
            }
        }
    }
    return 0;
}


int main(int argc, char** argv) {

    // init
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

        if (vm.count("invert-match"))
            invert_match = true;

        if (vm.count("ignore-case"))
            ignore_case = true;

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

    int check_occurences = 1;
    int find_line;
    if (filename_specified) {
        ifstream input(filename);
        if (!input.good()){
            cerr << "Incorrect filename!" << endl;
            return EXIT_FAILURE;
        }
        for(string line; getline(input,line);)
        {
            find_line = check_line(ref(line), ref(pattern), ignore_case, invert_match, regexp_specified, filename_specified);
            check_occurences += find_line;
        }

    } else {
        for (string line; getline(cin, line);) {
            find_line = check_line(ref(line), ref(pattern), ignore_case, invert_match, regexp_specified, filename_specified);
            check_occurences += find_line;
        }
    }

    if (check_occurences < 2){
        cout << "No matches found :(" << endl;
    }

    return 0;
}
