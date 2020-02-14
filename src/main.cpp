#include <iostream>
#include <rams.hpp>

template<> int* rams::ResourceHandler<int>::load(const std::string&){
    int* mine = new int;
    *mine = 5;
    return mine;
}

int main(int argc, char** argv){
    std::cout << "Hello World!" << std::endl;
    rams::Resource<int> mint("Baba");
    {
        rams::Resource<int> yint("Baba");
    }
    std::cout << mint() << std::endl;
    return EXIT_SUCCESS;
}