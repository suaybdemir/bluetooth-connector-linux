#ifndef SCANBLUETOOTH_DEVICES
#define SCANBLUETOOTH_DEVICES

#include <iostream>
#include <fstream>
#include <memory>
#include <string>

#include "../con.h"
#include "../connectors.h"

using namespace std;

CONNECTION_CONNECTORS connector;

shared_ptr<bool> retry = std::make_shared<bool>(false);

class ScanBluetoothDevices : public Con{
    
    public:
       
    void scanBluetoothDevices() {
        
        {//Scan Operations
        cout<<"Scan Started"<<endl;    

        connector.db.open("db.txt",ios::in | ios::out);//Lets create one

        system("hcitool scan > db.txt");//Lets write to the text file
        }

    connector.db.seekg(0, ios::end);
    bool empty = connector.db.tellg() == 0;
    connector.db.seekg(0, ios::beg);

    // if(empty==0){
    // {//Options as to decide what will do?
    // cout<<"1-Retry "<<endl;
    // cout<<"2-Exit"<<endl;
    // int option;
    // cin>>option;

    // if(option==1)
    // {
    // *retry = true;
    // }
    // else if(option==2)
    // {
    // return;
    // }
    // else
    // {
    // cout<<"1-Retry "<<endl;
    // cout<<"2-Exit"<<endl;
    // }
    // }
    // }



    if(*retry)
    {
        scanBluetoothDevices();
        return;
    }


    cout<<"----------Select-an-option---------"<<endl;
    {//Options as to choose MAC address
    connector.db.clear(); // as to clearly run this program (cleared any error flags!)
    connector.db.seekg(0,ios::beg); // cursor came to begin
    int i = 0;
    while(getline(connector.db,connector.line))// Get next line and write to the console

        if(i>=1)
        {
            cout<<i<<"- "<<connector.line<<endl;// Write line by line
        }
        i++;
    }
    cout<<"<-------*Appropriate-Bluetooth-Devices( to exit 0)------> "<<endl;//choose one option
    
    cin>>connector.lNum;
    if(connector.lNum==0)
    {
        return;
    }


    connector.lNum++;
    int count = 1;


    while(getline(connector.db,connector.line))// Get selected line
    {
        if(connector.lNum==count)
        {
            connector.temp = connector.line;
            cout<<connector.line<<endl;
            break;
        }
        count++;
    }

    for(int i = 0;i< connector.temp.length()-17 ; i++)// Add selected MAC address
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
};
#endif 
