#include <iostream>
#include <cstdlib>
#include <string>
using namespace std;

string address;

void scan() {

    system("hcitool scan");
    cout << "Write the Mac address" << endl;
    cin>>address;
}

void pairing() {
    system(("bluetoothctl pair " + address).c_str());
    system("exit");
}

void connect() {
    system(("bluetoothctl connect " + address).c_str());
    system("exit");
}

void disconnect() {
    system(("bluetoothctl disconnect " + address).c_str());
    system("exit");
}

int main(void) {
    system("rfkill unblock 0");

    cout << " Welcome to ToothDroid" << endl;

    for (;;) {
        cout << "----------What do you wanna do ?---------" << endl;
        cout << "-----------------------------------------" << endl;
        cout << "-----------------------------------------" << endl;
        cout << "          1-Scanning Devices             " << endl;
        cout << "          2-Pair Device               " << endl;
        cout << "          3-Connect to Device            " << endl;
        cout << "          4-Disconnect                   " << endl;
        cout << "          5-Exit                         " << endl;
        int num;
        cin >> num;
        switch (num) {
            case 1:
                scan();
                break;
            case 2:
                pairing();
                break;
            case 3:
                connect();
                break;
            case 4:
                disconnect();
                break;
            case 5:
                return 0; // Exiting the program
            default:
                break;
        }
    }
    return 0;
}
