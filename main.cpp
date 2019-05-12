#include "pf.h"

int main(int argc, char **argv)
{
    std::string ret = pf::format("%s:%d", argc > 1 ? argv[1] : "none", 3);
    printf("%s\n", ret.c_str());
    return 0;
}



