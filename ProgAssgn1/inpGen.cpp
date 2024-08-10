#include <iostream>
#include <fstream>
#include <cstdlib>
using namespace std;

int main()
{
    int N, K;

    cout << "Enter matrix size and number of threads" << endl;
    cin >> N >> K;

    ofstream outfile("inp.txt");
    if (!outfile) 
    {
        cerr << "Error: Unable to open file." << endl;
        return 1;
    }

    outfile << N << " " << K << endl;

    for(int i = 0; i < N; ++i) 
    {
        for (int j = 0; j < N; ++j)
            outfile << rand() % 100000 << " ";
        outfile << endl;
    }

    outfile.close();
}