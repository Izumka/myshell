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

using namespace std;
namespace po = boost::program_options;
namespace fs = boost::filesystem;


int main(int argc, char** argv) {

    // init
    bool recursively_mode = false;
    vector<string> paths;

    // parse command line args
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help,h",
             "mymkdir [-h|--help] [-f] [<file1> <file2> | <file1> <file2> <file3>... <dir> | <dir_or_file_1> <dir_or_file_2> <dir_or_file_3>... <dir>] – copy files or dirs")
            (",p", "Recursively copy all dirs and files: dir_or_file_1> <dir_or_file_2> <dir_or_file_3> ... <dir>");
    po::variables_map vm;


    try {
        auto parsed = po::command_line_parser(argc, argv).options(desc).run();
        paths = po::collect_unrecognized(parsed.options, po::include_positional);

        po::store(parsed, vm);
        po::notify(vm);

        // Print help and version of the program
        if (vm.count("help")) {
            std::cout << desc << "\n";
            return EXIT_SUCCESS;
        }

        if (vm.count("-p")) {
            recursively_mode = true;
        }


    } catch (po::error &e) {
        cerr << "ERROR: " << e.what() << std::endl;
        cerr << desc << std::endl;
        return EINVAL;
    }

//code place

    if(paths.size() == 0)
    {
        cout << "You didn`t specify the directory. \n";
        return 1;
    }

    if(recursively_mode)
    {
        fs::create_directories(fs::path(paths[0]));
    }
    else
    {
        try
        {
            fs::create_directory(fs::path(paths[0]));

        } catch(const fs::filesystem_error& e)
        {
            cout << "Hey you need recursive mode, use \"-p\" \n";
            return 1;
        }

    }

    return 0;
}
