#include <iostream>
#include <cstdlib>
#include <string>
#include <sstream>
#include <fstream>
#include "con.h"
#include "Entities/connection.h"
#include "Entities/pair.h"
#include "Entities/scanBluetoothDevices.h"
#include "mainScreen.h"
using namespace std;

int main(void) {

Con *con = Con::getInstance();

system("rfkill unblock 1");
system("rfkill unblock 0");

cout << "          Welcome to ToothDroid          " << endl;
cout<<endl;
cout << "----------What-do-you-wanna-do-?---------" << endl;
cout << "-----------------------------------------" << endl;
cout << "-----------------------------------------" << endl;
cout << "----------1-Scan-------------------------" << endl;
cout << "----------2-Disconnect-Through-Current---" << endl;
cout << "----------3-Exit-------------------------" << endl;
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
Connection disconnect;
disconnect.disconnect();
break;
case 3:
system("rm conf/db.txt");
return 0; 
default:
break;
}

screen();

return 0;
}
