#ifndef AVERAGER_H
#define AVERAGER_H

class Averager {
protected:
    unsigned long samples;
    double net;
public:
    Averager();
    void addSample(double);
    double getMean();
};

#endif