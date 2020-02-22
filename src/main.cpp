// author: Aaron Hornby
// ucid:   10176084

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "program.h"

//NOTE: apparently this is the proper way to forward declare namespaced-functions (you can't do "int wave_tool::program(int argc, char *argv[]);")
namespace wave_tool {
    int program(int argc, char *argv[]);
}

// reminder: argv[0] usually contains the executable name, argv[argc] is always a null pointer
int main(int argc, char *argv[]) {
    // run user-defined program...
    int const programResult = wave_tool::program(argc, argv);
    return programResult;
}

namespace wave_tool {
    // user-defined program...
    int program(int argc, char *argv[]) {
        // handle cmd-line args/options...

        // execute the rest of your program...
        Program program;
        bool const programResult = program.start();

        return programResult ? EXIT_SUCCESS : EXIT_FAILURE;
    }
}
