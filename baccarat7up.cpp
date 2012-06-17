#include "baccarat7up.h"

Hand::Hand() : vector<int>() {  
}

int Hand::getTotal() {
    int total=0;
    for (vector<int>::iterator iter=this->begin(); iter!=this->end(); iter++) {
        total += *iter;
    }
    return (total%10);
}

bool Hand::isNatural() {
    return (this->size() == 2) && (this->getTotal() >= 8);
}

// assume banker empty, player initialized with a 7
void Rules7Up::deal(Hand &player, Hand &banker, CSM &shoe) {
    // deal 1st banker card
    banker.push_back(shoe.deal());
    // deal 2nd player card
    player.push_back(shoe.deal());
    // deal 2nd banker card
    banker.push_back(shoe.deal());
    if (!player.isNatural() && !banker.isNatural()) {
        if (player.getTotal() <= 5) {
            // player draws
            player.push_back(shoe.deal());
        }
        if (player.size() == 2) {
            // player did not draw
            if (banker.getTotal() <= 5) {
                // banker draws
                banker.push_back(shoe.deal());
            }
        } else {
            // player drew
            int playerDraw = player[2];
            switch (banker.getTotal()) {
                case 0: case 1: case 2:
                    banker.push_back(shoe.deal());
                    break;
                case 3:
                    if (playerDraw != 8) {
                        banker.push_back(shoe.deal());
                    }
                    break;
                case 4:
                    if ((playerDraw >= 2) && (playerDraw <= 7)) {
                        banker.push_back(shoe.deal());
                    }
                    break;
                case 5:
                    if ((playerDraw >= 4) && (playerDraw <= 7)) {
                        banker.push_back(shoe.deal());
                    }
                    break;
                case 6:
                    if ((playerDraw >= 6) && (playerDraw <= 7)) {
                        banker.push_back(shoe.deal());
                    }
                    break;
            }
        }
    }
}

double Rules7Up::playerOutcome(Hand &player, Hand &banker) {
    int playerTotal = player.getTotal();
    int bankerTotal = banker.getTotal();
    if (playerTotal > bankerTotal) {
        return (playerTotal == 7 ? 0.5 : 1.0);
    } else if (playerTotal < bankerTotal) {
        return -1;
    } else {
        return 0;
    }
}

double Rules7Up::bankerOutcome(Hand &player, Hand &banker) {
    int playerTotal = player.getTotal();
    int bankerTotal = banker.getTotal();
    if (bankerTotal > playerTotal) {
        return (bankerTotal == 7 ? 1.8 : 1.0);
    } else if (bankerTotal < playerTotal) {
        return -1;
    } else {
        return 0;
    }
}

double Rules7Up::super7s(Hand &player, Hand &banker) {
    int sevens=0;
    for (vector<int>::iterator iter=player.begin(); iter!=player.end(); iter++) {
        if (*iter == 7) sevens++;
    }
    for (vector<int>::iterator iter=banker.begin(); iter!=banker.end(); iter++) {
        if (*iter == 7) sevens++;
    }
    switch (sevens) {
        case 6: return 700;
        case 5: return 70;
        case 4: return 17;
        case 3: return 5;
        case 2: return 2;
        default: return -1;
    }
}

CSM::CSM(int minDepth) {
    minBufferDepth = minDepth;
    minBufferDepth = minDepth;
    for (int i=0; i<6; i++) {
        for (int j=1; j<=9; j++) {
            for (int k=0; k<4; k++) reservoir.push_back(j);
        }
        for (int k=0; k<16; k++) reservoir.push_back(0);
    }
}

int CSM::deal() {
    int card, pos;
    while (buffer.size() < 1+minBufferDepth) {
        pos = reservoir.size()*random.genrand64_real2();
        buffer.insert(buffer.end(), reservoir[pos]);
        reservoir.erase(reservoir.begin()+pos);
    }
    card = buffer[0];
    buffer.erase(buffer.begin());
    return card;
}

void CSM::muck(int card) {
    reservoir.push_back(card);
}

ShuffleMaster126::ShuffleMaster126(int minDepth, int maxDepth) : CSM(minDepth) {
    maxCardsPerSlot = maxDepth;
    // shuffle in 6 decks
    for (int i=0; i<6; i++) {
        for (int j=1; j<=9; j++) {
            for (int k=0; k<4; k++) muck(j);
        }
        for (int k=0; k<16; k++) muck(0);
    }
}

void ShuffleMaster126::muck(int card) {
    int slotNum, pos;
    // get random slot
    slotNum = NUM_SLOTS*random.genrand64_real2();
    while (slots[slotNum].size() > maxCardsPerSlot) {
        slotNum = NUM_SLOTS*random.genrand64_real2();
    }
    // insert card into random position in slot
    pos = slots[slotNum].size()*random.genrand64_real2();
    slots[slotNum].insert(slots[slotNum].begin()+pos,card);
}

int ShuffleMaster126::deal() {
    int card;
    while (buffer.size() < (1+minBufferDepth)) {
        dropSlot();
    }
    card = buffer[0];
    buffer.erase(buffer.begin());
    return card;
}

void ShuffleMaster126::dropSlot() {
    // get random slot
    int slotNum = NUM_SLOTS*random.genrand64_real2();
    // drop into buffer
    buffer.insert(buffer.end(), slots[slotNum].begin(), slots[slotNum].end());
    slots[slotNum].clear();
}
