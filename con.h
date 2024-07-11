#ifndef CON_H
#define CON_H
#include <iostream>

const std::string* address;

class Con{
    private:
        static Con* instance ;

    public:
        static Con* getInstance(){
            
            if(!instance)
            {
                instance = new Con();
            }
            return instance;
        }

    void scanBluetoothDevices();
    void pairing();
    void connect();
    void disconnect();
};

Con *Con::instance = nullptr;

#endif