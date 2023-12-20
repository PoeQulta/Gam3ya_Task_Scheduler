#include <iostream>
#include <queue>
#include <thread>
#include <random>
#include <semaphore.h>
#include <unistd.h>
#define NUM_PROC 100
#define TimeStep 500
using namespace std;
sem_t cpuMutex;
int TerminatedCount;
class process
{
    public:
        int id;
        int burstTimeLeft;
        int initBurstTime;
        process(int id,int burstTime)
        {
            this->id = id;
            this->burstTimeLeft = burstTime;
            this->initBurstTime = burstTime;
        }
        int Execute(int Q)
        {
            this->burstTimeLeft-=Q;
            return (this->burstTimeLeft<=0);
        }
};
class QueueLvl
{
    public:
    queue<process> intQueue;
    int Q;
    QueueLvl *nxtLvl;
    QueueLvl *prevLvl;
    int isFull;
    sem_t full;
    sem_t empty;
    int sleepTime;
    QueueLvl(int q, int size, int freq, QueueLvl *prev, QueueLvl *nxt)
    {
        this->Q = q;
        this->nxtLvl = nxt;
        this->prevLvl = prev;
        this->sleepTime = ((float)100/freq) * TimeStep;
        this->isFull = 0;
        sem_init(&full,0,0);
        sem_init(&empty,0,size);
    }
    void enqueue(process proc)
    {
        int val;
        sem_wait(&empty);
            intQueue.push(proc);
        sem_post(&full);
        sem_getvalue(&empty,&val);
        isFull = !val;
    }
    void Run()
    {
        while(1)
        {
            int TaskDone;
            sem_wait(&full);
            sem_wait(&cpuMutex);
            process Temp = intQueue.front();
            intQueue.pop();
            TaskDone = Temp.Execute(Q);
            if(nxtLvl == NULL)
            {
                TaskDone = 1;
                cout << "Task in FCFS" << endl;
            } 
            if(TaskDone)
            {
                TerminatedCount++;
                cout << "Process w id: " << Temp.id << " Finished with initial Burst time: " << Temp.initBurstTime << " Finished: " << TerminatedCount <<  endl; 
            }
            sem_post(&cpuMutex);
            sem_post(&empty);
            if(!TaskDone)
            {
                if(prevLvl == NULL || (rand()%2) || prevLvl->isFull)
                    nxtLvl->enqueue(Temp);
                else
                    prevLvl->enqueue(Temp);
            }  
            usleep(sleepTime);
        }
    }
};


int main()
{
    TerminatedCount = 0;
    sem_init(&cpuMutex,0,1);
    random_device rd; // obtain a random number from hardware
    mt19937 gen(rd()); // seed the generator Twister pseudoRandom Generator
    uniform_int_distribution<> distr(2, 50); // define the range
    QueueLvl fcfs = QueueLvl(0,30,20,NULL,NULL);
    QueueLvl lvl2 = QueueLvl(16,20,30,NULL,NULL);
    QueueLvl lvl1 = QueueLvl(8,10,50,NULL,NULL);
    lvl1.nxtLvl = &lvl2;
    lvl2.prevLvl = &lvl1;
    lvl2.nxtLvl = &fcfs;
    fcfs.prevLvl = &lvl2;
    std::thread thread_1(&QueueLvl::Run, &lvl1);
    std::thread thread_2(&QueueLvl::Run, &lvl2);
    std::thread thread_3(&QueueLvl::Run, &fcfs);
    for (int i = 0; i < NUM_PROC; i++)
    {
        lvl1.enqueue(process(i,distr(gen)));
    }  
    thread_1.join();
    thread_2.join();
    thread_3.join();
}