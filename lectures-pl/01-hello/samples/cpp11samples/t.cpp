#include <vector>
#include <algorithm>
#include <numeric>
#include <future>
#include <iostream>
#include <unistd.h>

int main() {
    std::cout << "S\n";
    auto f = []()->int{std::cout << "async\n"; sleep(2);std::cout << "asyncOK\n";return 10;};
    auto handle = std::async(std::launch::async, f);
    std::cout << "T\n";
    std::cout << "ret: " << handle.get();

}
