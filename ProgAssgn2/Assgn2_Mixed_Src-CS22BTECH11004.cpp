#include <iostream>
#include <fstream>
#include <vector>
#include <chrono>
#include <thread>
#include <pthread.h>

using namespace std;

void multiplyMatrices(const vector<vector<int>>& A, vector<vector<int>>& result, int startRow, int endRow, int jump) {
    for (int i = startRow; i <= endRow; i += jump) {
        for (int j = 0; j < A.size(); ++j) {
            for (int k = 0; k < A.size(); ++k) {
                result[i][j] += A[i][k] * A[k][j];
            }
        }
    }
}

void setAffinity(int core_id) {
    pthread_t current_thread = pthread_self();
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset);
}

int main() {
    int N, K, C, BT; 

    ifstream infile("inp.txt");
    if (!infile) {
        cerr << "Error: Unable to open input file." << endl;
        return 1;
    }

    infile >> N >> K >> C >> BT; 

    vector<vector<int>> matrixA(N, vector<int>(N));
    vector<vector<int>> result(N, vector<int>(N, 0));

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j)
            infile >> matrixA[i][j];
    }

    infile.close();

    vector<thread> threads;
    auto startTime = chrono::high_resolution_clock::now();

    for (int i = 0; i < K; ++i) {
        int startRow = i * (N / K);
        int endRow = (i == K - 1) ? (N - 1) : ((i + 1) * (N / K) - 1);
        threads.emplace_back([startRow, endRow, &matrixA, &result, C, BT]() {
            setAffinity(BT); // Set CPU affinity
            multiplyMatrices(matrixA, result, startRow, endRow, 1); // Set jump to 1
        });
    }

    // Waiting for all threads to finish
    for (auto& thread : threads)
        thread.join();

    auto endTime = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime).count();

    ofstream outfile("out_mixed.txt");
    if (!outfile) {
        cerr << "Error: Unable to open output file." << endl;
        return 1;
    }

    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j)
            outfile << result[i][j] << " ";
        outfile << endl;
    }

    outfile << "Time taken: " << duration << " milliseconds" << endl;

    outfile.close();

    return 0;
}
