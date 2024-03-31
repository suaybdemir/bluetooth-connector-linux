#ifndef CON_H
#define CON_H
#include <iostream>

class Con{
    protected:

        std::string address;

    public:

        Con() : address(""){}
        void scanBluetoothDevices();
        void pairing();
        void connect();
        void disconnect();
};



#endif