#include "Parser.hpp"
#include <fstream>

// Use tutorials in: https://llvm.org/docs/tutorial/

int main (int argc, char *argv[])
{
    //if (argc < 2) return 1;

    //std::cout << argv[1] << std::endl;
    FILE *file = stdin; //fopen(argv[1], "r");
    if (file == NULL) {
        perror("Err opening file.");
        return 1;
    }

    Parser parser(file);

    if (!parser.Parse()) {
        return 1;
    }

    parser.Generate().print(llvm::outs(), nullptr);

    fclose(file);
    return 0;
}
