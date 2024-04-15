#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include "con.h"
#include "Entities/connection.h"
#include "Entities/pair.h"
#include "Entities/scanBluetoothDevices.h"
using namespace std;

void screen() {

    cout << "          Welcome to ToothDroid          " << endl;
    for (;;) {
        cout<<endl;
        cout << "----------What-do-you-wanna-do-?---------" << endl;
        cout << "-----------------------------------------" << endl;
        cout << "-----------------------------------------" << endl;
        cout << "----------1-Scanning Devices-------------" << endl;
        cout << "----------2-Pair-Device------------------" << endl;
        cout << "----------3-Connect-to-Device------------" << endl;
        cout << "----------4-Disconnect-------------------" << endl;
        cout << "----------5-Exit-------------------------" << endl;
        cout << "-----------------------------------------" << endl;
        cout << "-----------------------------------------" << endl;
     
        cout<<endl;
        int num;
        cin >> num;
        switch (num) {
            case 1:
                ScanBluetoothDevices scan;
                scan.scanBluetoothDevices();
                break;
            case 2:
                Pair pair;
                pair.pairing();
                break;
            case 3:
                Connection connection;
                connection.connect();
                break;
            case 4:
                Connection disconnect;
                disconnect.disconnect();
                break;
            case 5:
                system("rm Entities/db.txt");
                break;
            default:
                break;
        }
    }
}
