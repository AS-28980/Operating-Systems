#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>
using namespace std;

std::atomic<int> C(0);
std::vector<std::vector<int>> matrix;
std::vector<std::vector<int>> result;
int N, K, R;

std::mutex mtx;

void multiplyRows(int startRow, int endRow) 
{
    for (int i = startRow; i <= endRow; ++i) 
    {
        for (int j = 0; j < N; ++j) 
        {
            int sum = 0;
            for (int k = 0; k < N; ++k) 
                sum += matrix[i][k] * matrix[k][j];
            result[i][j] = sum;
        }
    }
}

void criticalSection() 
{
    int startRow, endRow;
    while (true) 
    {
        int current = C.fetch_add(R);
        if (current >= N) break; 

        startRow = current;
        endRow = min(N - 1, startRow + R - 1);

        multiplyRows(startRow, endRow);
    }
}

int main() 
{
    std::ifstream infile("inp.txt");
    std::ofstream outfile("out_Atomic.txt");

    if (!infile) 
    {
        std::cerr << "Error: Unable to open input file." << std::endl;
        return 1;
    }

    if (!outfile) 
    {
        std::cerr << "Error: Unable to open output file." << std::endl;
        return 1;
    }

    infile >> N >> K >> R;
    matrix.resize(N, std::vector<int>(N));
    result.resize(N, std::vector<int>(N));

    for (int i = 0; i < N; ++i) 
    {
        for (int j = 0; j < N; ++j) 
            infile >> matrix[i][j];
    }

    infile.close();

    auto startTime = chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < K; ++i)
        threads.push_back(std::thread(criticalSection));

    for (auto& thread : threads) 
        thread.join();

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

    for (int i = 0; i < N; ++i) 
    {
        for (int j = 0; j < N; ++j) 
            outfile << result[i][j] << " ";
        outfile << std::endl;
    }

    outfile << "Time taken: " << duration << " milliseconds" << endl;

    outfile.close();

    return 0;
}
