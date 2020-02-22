/*
 * Sample boilerplate taken from CPSC453 at the University of Calgary
 * Main class for user-defined program
 * Created on: Sep 10, 2018
 * Author: John Hall
 * Modifications by: Aaron Hornby (10176084)
 */

#ifndef PREFIX_PROGRAM_H_
#define PREFIX_PROGRAM_H_

//#include "image-buffer.h"

struct GLFWwindow;

namespace prefix {
    class Program {
        public:
            Program();
            ~Program();

            // runs the user defined program (including render loop)
            bool start();
        private:
            //ImageBuffer image;
            GLFWwindow *window = NULL;

            // prints system specs to the console
            void queryGLVersion();

            // initializes GLFW and creates the window
            bool setupWindow();
    };

    // functions passed to GLFW to handle errors and keyboard input
    //NOTE: GLFW requires them to not be member functions of a class
    void errorCallback(int error, char const* description);
    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
}

#endif // PREFIX_PROGRAM_H_
