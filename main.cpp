#include "baccarat7up.h"
#include "averager.h"
#include <map>

#define MAX_WINDOW_DEPTH 100

void usage() {
    printf("sevenUp [-m minBufferDepth] [-d maxSlotDepth] [-v 0|1|2] [-w windowSize]\n");
    printf("  -m minBufferDepth sets the minimum buffer depth threshold in exit chute\n");
    printf("  -d maxSlotDepth sets the maximum cards placed in any CSM slot\n");
    printf("  -v 0 = print player stats, 1 = print banker stats, 2 = print Super-7's stats\n");
    printf("  -w windowSize sets the number of cards used in the count (last windowSize cards out of CSM)\n");
}

static int history[MAX_WINDOW_DEPTH];
static int wp=0, windowSize=20;     // default value

// count values for Banker
int bankerWeight(int card) {
    switch (card) {
        case 0: return 1;
        case 1: return 2;
        case 2: return 2;
        case 3: case 4: case 5: case 6: case 8: case 9: return -1;
        case 7: return -2;
        default: return 0;
    }
}

// count values for Player
int playerWeight(int card) {
    switch (card) {
        case 0: return 0;
        case 1: return -2;
        case 2: return -2;
        case 3: case 4: case 5: return 0;
        case 6: case 7: case 8: case 9: return 1;
        default: return 0;
    }
}

int sevensWeight(int card) {
    return (card == 7) ? -12 : 1;
}

// returns windowed count for given weight function
int count(int (*weight)(int)) {
    int total = 0;
    for (int i=0; i<windowSize; i++) {
        total += weight(history[i]);
    }
    return total;
}

void addStats(double value, int count, map<int,Averager> &stats) {
    map<int,Averager>::iterator iter;
    iter = stats.find(count);
    if (iter == stats.end()) {
        stats.insert(pair<int,Averager>(count,Averager())).first->second.addSample(value);
    } else {
        iter->second.addSample(value);
    }
}

void printStats(map<int,Averager> &stats) {
    for (map<int,Averager>::iterator iter=stats.begin(); iter!=stats.end(); iter++) {
        printf("%+2d: %+8.6f\n", iter->first, iter->second.getMean());
    }
}

// add cards to history, then muck into CSM
void countAndMuck(CSM &csm, vector<int>::iterator begin, vector<int>::iterator end) {
    for (vector<int>::iterator iter=begin; iter!=end; iter++) {
        csm.muck(*iter);
        history[wp] = *iter;
        wp = (wp+1)%windowSize;
    }
}

int main(int argc, const char * argv[])
{
    int minBufferDepth=20, maxCardsPerSlot=30;      // default values
    unsigned long hands=0;
    double playerNet=0, bankerNet=0, sevensNet=0;
    double playerOutcome, bankerOutcome, sevensOutcome;
    Hand player, banker;
    int bankerCount, playerCount, sevensCount;
    // per-count stats
    map<int,Averager> bankerStats, playerStats, sevensStats;
    
    bool verbose[3];
    for (int i=0; i<3; i++) verbose[i] = false;
    int c;
    while ((c = getopt(argc, (char * const *)argv, "hm:d:v:w:")) != EOF) {
        switch (c) {
            case 'h': usage(); exit(0);
            case 'm': minBufferDepth = atoi(optarg); break;
            case 'd': maxCardsPerSlot = atoi(optarg); break;
            case 'v': verbose[atoi(optarg)] = true; break;
            case 'w': windowSize = atoi(optarg); break;
        }
    }
    
    printf("using ShuffleMaster126 CSM model with (%d minBufferDepth, %d maxCardsPerSlot), and a windowSize of %d cards\n", minBufferDepth, maxCardsPerSlot, windowSize);
    ShuffleMaster126 csm(minBufferDepth,maxCardsPerSlot);
    
    // player always starts with a 7
    player.push_back(7);
    while (true) {
        bankerCount = count(&bankerWeight);
        playerCount = count(&playerWeight);
        sevensCount = count(&sevensWeight);
        // deal hand
        Rules7Up::deal(player,banker,csm);
        // resolve hand
        bankerOutcome = Rules7Up::bankerOutcome(player,banker);
        playerOutcome = Rules7Up::playerOutcome(player,banker);
        sevensOutcome = Rules7Up::super7s(player,banker);
        addStats(bankerOutcome, bankerCount, bankerStats);
        addStats(playerOutcome, playerCount, playerStats);
        addStats(sevensOutcome, sevensCount, sevensStats);
        bankerNet += bankerOutcome;
        playerNet += playerOutcome;
        sevensNet += sevensOutcome;
        // muck cards
        countAndMuck(csm, banker.begin(), banker.end());
        banker.clear();
        countAndMuck(csm, player.begin()+1, player.end());
        // leave the starting 7 in the player hand
        player.erase(player.begin()+1,player.end());
        hands++;
        if (hands%10000000 == 0) {
            printf("hands: %ld, player EV: %+8.6f, banker EV: %+8.6f, sevens EV: %+8.6f\n", hands, playerNet/(double)hands, bankerNet/(double)hands, sevensNet/(double)hands);
            if (verbose[0]) {printStats(playerStats); printf("\n");}
            if (verbose[1]) {printStats(bankerStats); printf("\n");}
            if (verbose[2]) {printStats(sevensStats); printf("\n");}
        }
    }
    return 0;
}