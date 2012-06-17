#ifndef BACCARAT_7UP_H
#define BACCARAT_7UP_H

#include <vector>
#include "mersenne64.h"

using namespace std;

#define NUM_SLOTS 40

class Hand : public vector<int> {
public:
    Hand();
    int getTotal();
    bool isNatural();
};

class CSM {
protected:
    Mersenne64 random;
    vector<int> buffer;
    int minBufferDepth;
    vector<int> reservoir;
public:
    CSM(int);
    virtual int deal();
    virtual void muck(int);
};

class ShuffleMaster126 : public CSM {
protected:
    vector<int> slots[NUM_SLOTS];
    void dropSlot();
    int maxCardsPerSlot;
public:
    ShuffleMaster126(int minDepth, int maxDepth);
    virtual int deal();
    virtual void muck(int);
};

class Rules7Up {
public:
    static void deal(Hand &player, Hand &banker, CSM &shoe);
    static double bankerOutcome(Hand &player, Hand &banker);
    static double playerOutcome(Hand &player, Hand &banker);
    static double super7s(Hand &player, Hand &banker);
};

#endif