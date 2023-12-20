#include <iostream>
#include <queue>
#include <thread>
#include <random>
#include <semaphore.h>
#define NUM_PROC 20
#define TimeStep 100
using namespace std;
sem_t cpuMutex;

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
    QueueLvl nxtLvl;
    QueueLvl prevLvl;
    sem_t full;
    sem_t empty;
    sem_t queueMutex;
    int sleepTime;
    QueueLvl(int q, int size, int freq, QueueLvl prev, QueueLvl nxt)
    {
        this->Q = q;
        this->nxtLvl = nxt;
        this->prevLvl = prev;
        this->sleepTime = ((float)100/freq) * TimeStep;
        sem_init(&full,0,0);
        sem_init(&empty,0,size);
        sem_init(&queueMutex,0,1);
    }
    void enqueue(process proc)
    {
        sem_wait(&empty);
        sem_wait(&queueMutex);
            intQueue.push(proc);
        sem_post(&queueMutex);
        sem_post(&full);
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
                TaskDone = 1;
            if(TaskDone)
            {
                cout << "Process w id: " << Temp.id << " Finished with initial Burst time: " << Temp.initBurstTime << endl; 
            }
            sem_post(&cpuMutex);

            if(!TaskDone)
            {
                if(prevLvl == NULL || (rand() %2))
                    nxtLvl.enqueue(Temp);
                else
                    prevLvl.enqueue(Temp);
            }
            sem_post(&empty);
            usleep(sleepTime);
        }
    }
};


int main()
{
    sem_init(&cpuMutex,0,1);
    random_device rd; // obtain a random number from hardware
    mt19937 gen(rd()); // seed the generator Twister pseudoRandom Generator
    uniform_int_distribution<> distr(2, 50); // define the range
    QueueLvl fcfs = QueueLvl(0,30,20,NULL,NULL);
    QueueLvl lvl2 = QueueLvl(16,20,30,NULL,NULL);
    QueueLvl lvl1 = QueueLvl(8,10,50,NULL,NULL);
    lvl1.nxtLvl = lvl2;
    lvl2.prevLvl = lvl1;
    lvl2.nxtLvl = fcfs;
    std::thread thread_1(&QueueLvl::Run, &lvl1);
    std::thread thread_2(&QueueLvl::Run, &lvl2);
    std::thread thread_2(&QueueLvl::Run, &fcfs);
    process e;
    for (int i = 0; i < NUM_PROC; i++)
    {
        e = process(i,distr(gen));
        lvl1.enqueue(e);
    }   
}