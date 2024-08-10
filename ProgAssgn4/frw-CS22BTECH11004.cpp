#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <ctime>
#include <random>
#include <semaphore.h>

using namespace std;

int nw, nr, kw, kr;
double muCS, muREM;
sem_t resource, rmutex, serviceQueue;
int readcount = 0;

vector<double> readerWaitTimes, writerWaitTimes;

void randCSTime(double muCS) 
{
    default_random_engine generator(chrono::system_clock::now().time_since_epoch().count());
    exponential_distribution<double> distribution(1.0 / muCS);
    this_thread::sleep_for(chrono::milliseconds((int)distribution(generator)));
}

void randRemTime(double muREM) 
{
    default_random_engine generator(chrono::system_clock::now().time_since_epoch().count());
    exponential_distribution<double> distribution(1.0 / muREM);
    this_thread::sleep_for(chrono::milliseconds((int)distribution(generator)));
}

void writeToLog(const string& logEntry) 
{
    ofstream logfile("FairRW-log.txt", ios::app); // Open the file in append mode
    if (logfile.is_open()) 
    {
        logfile << logEntry << endl;
        logfile.close();
    } 
    else 
        cerr << "Error: Unable to open log file!" << endl;
}

void calculateAndWriteAverageTimes() 
{
    double totalReaderWaitTime = 0, totalWriterWaitTime = 0;
    for (double time : readerWaitTimes)
        totalReaderWaitTime += time;
    for (double time : writerWaitTimes)
        totalWriterWaitTime += time;

    double avgReaderWaitTime = (readerWaitTimes.empty()) ? 0 : totalReaderWaitTime / readerWaitTimes.size();
    double avgWriterWaitTime = (writerWaitTimes.empty()) ? 0 : totalWriterWaitTime / writerWaitTimes.size();

    ofstream avgFile("Average_Time_frw.txt");
    if (avgFile.is_open()) 
    {
        avgFile << "Average Wait Time for Reader Threads: " << avgReaderWaitTime << " ms" << endl;
        avgFile << "Average Wait Time for Writer Threads: " << avgWriterWaitTime << " ms" << endl;
        avgFile.close();
    } 
    else 
        cerr << "Error: Unable to open Average_Time_frw.txt file!" << endl;
}

void calculateAndWriteWorstTime()
{
    double worstReaderWaitTime = 0, worstWriterWaitTime = 0;
    for(double time : readerWaitTimes)
    {
        if(time > worstReaderWaitTime)
            worstReaderWaitTime = time;
    }
    for(double time : writerWaitTimes)
    {
        if(time > worstWriterWaitTime)
            worstWriterWaitTime = time;
    }

    ofstream avgFile("Worst_Time_frw.txt");
    if (avgFile.is_open()) 
    {
        avgFile << "Worst Wait Time for Reader Threads: " << worstReaderWaitTime << " ms" << endl;
        avgFile << "Worst Wait Time for Writer Threads: " << worstWriterWaitTime << " ms" << endl;
        avgFile.close();
    } 
    else 
        cerr << "Error: Unable to open Worst_Time_frw.txt file!" << endl;
}

void reader(int id) 
{
    for (int i = 0; i < kr; i++) 
    {
        auto reqTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
        writeToLog(to_string(i+1) + "th CS request by Reader Thread " + to_string(id+1) + " at " + ctime(&reqTime));

        sem_wait(&serviceQueue);
        sem_wait(&rmutex);
        readcount++;
        if (readcount == 1)
            sem_wait(&resource);
        sem_post(&rmutex);
        sem_post(&serviceQueue);

        auto enterTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
        writeToLog(to_string(i+1) + "th CS Entry by Reader Thread " + to_string(id+1) + " at " + ctime(&enterTime));

        randCSTime(muCS); // Simulate thread executing in critical section

        auto exitTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
        writeToLog(to_string(i+1) + "th CS Exit by Reader Thread " + to_string(id+1) + " at " + ctime(&exitTime));

        sem_wait(&rmutex);
        readcount--;
        if (readcount == 0)
            sem_post(&resource);
        sem_post(&rmutex);

        randRemTime(muREM); // Simulate thread executing in Remainder Section

        auto reqTimePoint = chrono::system_clock::from_time_t(reqTime);
        auto enterTimePoint = chrono::system_clock::from_time_t(enterTime);
        auto waitDuration = chrono::duration_cast<chrono::milliseconds>(enterTimePoint - reqTimePoint).count();
        readerWaitTimes.push_back(waitDuration);
    }
}

void writer(int id) 
{
    for (int i = 0; i < kw; i++) 
    {
        auto reqTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
        writeToLog(to_string(i+1) + "th CS request by Writer Thread " + to_string(id+1) + " at " + ctime(&reqTime));

        sem_wait(&serviceQueue);
        sem_wait(&resource);

        auto enterTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
        writeToLog(to_string(i+1) + "th CS Entry by Writer Thread " + to_string(id+1) + " at " + ctime(&enterTime));

        randCSTime(muCS); // Simulate thread executing in critical section

        auto exitTime = chrono::system_clock::to_time_t(chrono::system_clock::now());
        writeToLog(to_string(i+1) + "th CS Exit by Writer Thread " + to_string(id+1) + " at " + ctime(&exitTime));

        sem_post(&resource);
        sem_post(&serviceQueue);

        randRemTime(muREM); // Simulate thread executing in Remainder Section

        auto reqTimePoint = chrono::system_clock::from_time_t(reqTime);
        auto enterTimePoint = chrono::system_clock::from_time_t(enterTime);
        auto waitDuration = chrono::duration_cast<chrono::milliseconds>(enterTimePoint - reqTimePoint).count();
        writerWaitTimes.push_back(waitDuration);
    }
}

int main() {
    ifstream inFile("inp-params.txt");
    inFile >> nw >> nr >> kw >> kr >> muCS >> muREM;
    inFile.close();

    sem_init(&resource, 0, 1);
    sem_init(&rmutex, 0, 1);
    sem_init(&serviceQueue, 0, 1);

    thread readers[nr], writers[nw];

    for (int i = 0; i < nw; ++i)
        writers[i] = thread(writer, i);
    for (int i = 0; i < nr; ++i)
        readers[i] = thread(reader, i);

    for (int i = 0; i < nw; ++i)
        writers[i].join();
    for (int i = 0; i < nr; ++i)
        readers[i].join();

    sem_destroy(&resource);
    sem_destroy(&rmutex);
    sem_destroy(&serviceQueue);

    calculateAndWriteAverageTimes();
    calculateAndWriteWorstTime();

    return 0;
}