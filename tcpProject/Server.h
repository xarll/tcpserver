#include <iostream>

char* slice(char* s, int from, int to);
int startServer(std::string(*func)(std::string), const char* addresses, const char* port);