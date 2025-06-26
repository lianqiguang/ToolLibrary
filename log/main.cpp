#include <iostream>
#include "Logger.h"

using namespace std;
using namespace Zero::Logger;

int main()
{
    Logger::instance()->open("./test.log");
    Logger::instance()->level(Logger::DEBUG);
    Logger::instance()->max(200);
    // Logger::instance()->log(Logger::DEBUG, __FILE__, __LINE__, "hello world");
    // Logger::instance()->log(Logger::DEBUG, __FILE__, __LINE__, "name is %s, age is %d", "jack", 18);

    debug("hello world");
    debug("name is %s, age is %d", "jack", 18);
    info("info message");
    warn("warn message");
    error("error message");


    cout<< "Zero log" << std::endl;
    return 0;
}