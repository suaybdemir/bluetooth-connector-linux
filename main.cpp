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
        void scanBluetoothDevices();
        void pairing();
        void connect();
        void disconnect();
};

void Con::scanBluetoothDevices()
{
    address.clear(); // Firstly Clear Address

    cout<<"Scan Started"<<endl;
    system("hcitool scan > db.txt");

    fstream db;

    db.open("db.txt",ios::in | ios::out);
    if(db.fail())
    {
        cout<<"Error #001 Do you want to try again[y/n]?"<<endl;
        char ch;
        cin>>ch;
        if(ch=='y')
        {
            scanBluetoothDevices();
        }
    }

    int i = 0;

    cout<<"-------------Appropriate Bluetooth Devices!!------------------- "<<endl;

    string line;

    while(getline(db,line))
    {
        if(i>=1)
        {
            cout<<i<<"- "<<line<<endl;
        }
        i++;
    }

    cout<<"Select an option (to exit write -1)"<<endl;
    int lNum;
    cin>>lNum;

    lNum++;

    string temp;
    int count = 1;

    db.clear(); // as to clearly run this program (cleared any error flags!)
    db.seekg(0,ios::beg); // cursor came to begin

    while(getline(db,line))
    {
        if(lNum==count)
        {
            temp = line;
            cout<<line<<endl;
            break;
        }
        count++;
    }

    for(int i = 0;i< temp.length()-17 ; i++)
    {
        if(temp[i+2]== ':')
        {
            for(int j = 0; j<17 ; j++)
            {
                address += temp[i+j];
            }
            cout<<endl;
            break;
        }
    }

    cout<<"This is selected Bluetooth Device MAC address!!!"<<address<<endl;

    db.close();

    if(db.fail())
    {
        cout<<"Error #002 ( Not important! )"<<endl;
    }
}

void Con::pairing() {
    system(("bluetoothctl pair " + address).c_str());
    system("exit");
    
}

void Con::connect() {
    system(("bluetoothctl connect " + address).c_str());
    system("exit");
    
}

void Con::disconnect() {
    system("bluetoothctl disconnect ");
    system("exit");
    
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
        // cout << "----------[Selected Mac Address]---------" << endl;
        // cout << "---------"<<con.address<<"---------------" << endl;
        cout<<endl;
        int num;
        cin >> num;
        switch (num) {
            case 1:
                con.scanBluetoothDevices();
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
