#ifndef PAIR_H
#define PAIR_H

#include "../con.h"

class Pair : public Con{
    public:
        void pairing() {
             system(("bluetoothctl pair " + address).c_str());
             system("exit");
        }
};

#endif