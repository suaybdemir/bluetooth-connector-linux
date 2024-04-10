#ifndef CON_H
#define CON_H
#include <iostream>

std::string address;

class Con{
    public:

        static Con& getInstance(){
            static Con instance;
            return instance;
        }
        void scanBluetoothDevices();
        void pairing();
        void connect();
        void disconnect();

        
};


// class Con{
//     public:
//         static Con& getInstance()
//         {
//             static Con instance;
//             return instance;
//         }
//         void scanBluetoothDevices();
//         void pairing();
//         void connect();
//         void disconnect();
        
// };

#endif