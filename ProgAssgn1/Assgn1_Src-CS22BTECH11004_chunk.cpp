#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>

using namespace std;

void multiplyMatrices(const vector<vector<int>>& A, vector<vector<int>>& result, int startRow, int endRow) 
{
    for (int i = startRow; i <= endRow; ++i) 
    {
        for (int j = 0; j < A.size(); ++j) 
        {
            for (int k = 0; k < A.size(); ++k)
                result[i][j] += A[i][k] * A[k][j];
        }
    }
}

int main() 
{
    int N, K;

    ifstream infile("inp.txt");
    if (!infile) 
    {
        cerr << "Error: Unable to open input file." << endl;
        return 1;
    }

    infile >> N >> K;

    vector<vector<int>> matrixA(N, vector<int>(N));
    vector<vector<int>> result(N, vector<int>(N, 0));

    for (int i = 0; i < N; ++i) 
    {
        for (int j = 0; j < N; ++j) 
            infile >> matrixA[i][j];
    }

    infile.close();

    int chunkSize = N / K;
    vector<thread> threads;

    auto startTime = chrono::high_resolution_clock::now();

    for (int i = 0; i < K; ++i) {
        int startRow = i * chunkSize;
        int endRow = (i + 1) * chunkSize - 1;
        threads.emplace_back(multiplyMatrices, ref(matrixA), ref(result), startRow, endRow);
    }

    // Waiting for all threads to finish
    for (auto& thread : threads)
        thread.join();

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(endTime - startTime).count();

    ofstream outfile("out_chunk.txt");
    if (!outfile) 
    {
        cerr << "Error: Unable to open output file." << endl;
        return 1;
    }

    for (int i = 0; i < N; ++i) 
    {
        for (int j = 0; j < N; ++j)
            outfile << result[i][j] << " ";
        outfile << endl;
    }

    outfile << "Time taken: " << duration << " microseconds" << endl;

    outfile.close();

    return 0;
}
