#ifndef CON_H
#define CON_H
#include <iostream>

class Con{
    protected:
        std::string address;
    private:
        Con() : address(""){}
    public:
        static Con& getInstance()
        {
            static Con instance;
            return instance;
        }
        void scanBluetoothDevices();
        void pairing();
        void connect();
        void disconnect();
};

#endif