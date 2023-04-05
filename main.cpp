#include "window.h"

int main(int argc, char *argv[])
{
    std::cout<<"Hello World"<<std::endl; 

    std::unique_ptr<Window> m_window = std::make_unique<Window>();

    std::cout<<"Start"<<std::endl;
    int startResult = m_window->start();
    if(startResult != 0){
        return startResult;
    }

    std::cout<<"Loop"<<std::endl;
    int loopResult = m_window->loop();
    if(loopResult != 0){
        return loopResult;
    }

    std::cout<<"End"<<std::endl;
    return 0;
}