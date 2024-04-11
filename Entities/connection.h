#ifndef CONNECTION_H
#define CONNECTION_H

#include "../con.h"

class Connection : public Con{
    public:

        void connect() {
            system(("bluetoothctl connect " + address).c_str());
            system("exit");
        }
        void disconnect() {
            system("bluetoothctl disconnect ");
            system("exit");
        }
};

#endif