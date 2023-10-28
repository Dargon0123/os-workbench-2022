#include <iostream>
#include <cassert>

#define N 100000

using namespace std;

int main(int argc, char *argv[]) {
    char ch;
    int count = 0;
    
    assert(argc == 2);
    int limit = atoi(argv[1]);
    cout << "pc-check start!" << endl;
    while (1) {
        for (int i = 0; i < N; ++i) {
            std::cin >> ch;
            if (ch == '(') {
                count++;
            }
            else if (ch == ')') {
                count--;
            }
            assert (count >=0 && count <= limit);        
        }
        cout << N << " Ok!" << endl;
    }
}