#include <iostream>
#include <string>

#include "Server.h"


std::string dirFinder(std::string search)
{

    std::string result = "hello, " + search;
    return result;
}




int main()
{
    setlocale(LC_ALL, "rus");
    startServer(dirFinder, "127.0.0.1", "27015");
    return 0;
}