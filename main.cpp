#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
using namespace std;

class Con{
    protected:
        string address;
    public:
        Con() : address(""){}
        string& scan();
        string& pairing();
        string& connect();
        string& disconnect();
};

string& Con::scan()
{

    cout<<"Scan Started"<<endl;
    system("hcitool scan && hcitool scan > db.txt");

    cout<<"Do u wanna write MAC address[y/n]"<<endl;
    char c; 
    cin>>c;
    if(c=='y')
    {
        cout << "Write the Mac address" << endl;
        cin>>address;
    }
    return address;
}



string& Con::pairing() {
    system(("bluetoothctl pair " + address).c_str());
    system("exit");
    return address;
}

string& Con::connect() {
    system(("bluetoothctl connect " + address).c_str());
    system("exit");
    return address;
}

string& Con::disconnect() {
    system("bluetoothctl disconnect ");
    system("exit");
    return address;
}

int main(void) {

    Con con;


    for(int i=0 ; i<3; i++)
    {
        std::stringstream command;
        command << "rfkill unblock " << i;
        system(command.str().c_str());
    }
    

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
                con.scan();
                break;
            case 2:
                con.pairing();
                break;
            case 3:
                con.connect();
                break;
            case 4:
                con.disconnect();
                break;
            case 5:
                return 0; // Exiting the program
            default:
                break;
        }
    }
    return 0;
}
