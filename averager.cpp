#include "averager.h"

Averager::Averager() {
    samples = 0;
    net = 0;
}

void Averager::addSample(double value) {
    net += value;
    samples++;
}

double Averager::getMean() {
    return net/(double)samples;
}