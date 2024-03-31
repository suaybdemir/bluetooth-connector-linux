#include "con.h"
#include <iostream>
#include <fstream>
#include "connectors.h"
using namespace std;

void Con::scanBluetoothDevices()
{
    CONNECTION_CONNECTORS connector;
    address.clear(); // Firstly Clear Address

    cout<<"Scan Started"<<endl;    

    connector.db.open("conf/db.txt",ios::in | ios::out);

    system("hcitool scan > conf/db.txt");

    cout<<"<-------*Appropriate-Bluetooth-Devices*------> "<<endl;

    int i = 0;

    while(getline(connector.db,connector.line))
    {
        if(i>=1)
        {
            cout<<i<<"- "<<connector.line<<endl;
        }
        i++;
    }

    cout<<"Select an option (to exit write -1)"<<endl;
    cin>>connector.lNum;

    connector.lNum++;

    connector.db.clear(); // as to clearly run this program (cleared any error flags!)
    connector.db.seekg(0,ios::beg); // cursor came to begin

    int count = 1;

    while(getline(connector.db,connector.line))
    {
        if(connector.lNum==count)
        {
            connector.temp = connector.line;
            cout<<connector.line<<endl;
            break;
        }
        count++;
    }

    for(int i = 0;i< connector.temp.length()-17 ; i++)
    {
        if(connector.temp[i+2]== ':')
        {
            for(int j = 0; j<17 ; j++)
            {
                address += connector.temp[i+j];
            }
            cout<<endl;
            break;
        }
    }

    connector.db.close();
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