#include "con.h"
#include <iostream>
#include <fstream>
using namespace std;

void Con::scanBluetoothDevices()
{
    address.clear(); // Firstly Clear Address

    cout<<"Scan Started"<<endl;

    fstream db;
    

    db.open("conf/db.txt",ios::in | ios::out);
    system("hcitool scan > conf/db.txt");

    if(db.fail())
    {
        cout<<"Error #001 Occured! "<<endl;
        return;
    }

    int i = 0;

    cout<<"<-------*Appropriate-Bluetooth-Devices*------> "<<endl;

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

    db.close();
    if(db.fail())
    {
        cout<<"Error #002 Occured!( Not important )"<<endl;
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