#include <iostream>
#include <stdio.h>
#include <string.h>
#include <random>
#include <map>
#include <ctime>
#include <SQLAPI.h>


using namespace std;

class SQLServer {
public:
    string Host, Port;
    string User, Password;
    string Database;

    string getHost() { return Host; }
    string getPort() { return Port; }
    string getUser() { return User; }
    string getPassword() { return Password; }
    string getDatabase() { return Database; }

    SQLServer()
    {
        Host = "140.128.109.115";
        Port = "1433";
        User = "sa";
        Password = "s08490043";
    }

};

class Machine {
    string MachineId;
    map<string, string> MachineValue;
    string UpdateTime;

public:
    string getId()  { return MachineId; }
    map<string, string> getValue() { return MachineValue; }
    string getUpdateTime()
    {
        time_t rawtime = time(NULL);
        tm* timeInfo = localtime(&rawtime);
        char chstr[80];

        strftime(chstr, sizeof(chstr),"%Y-%m-%d %H:%M:%S",timeInfo);

        return string(chstr);
    }
};

void createDatabase();
void createTable();
void insertData();
void deleteData();
struct tm* getNow();
string tm_to_string(struct tm* tm);
Machine generateData();


int main()
{
    try{
        struct tm* start = getNow();

        while(true){
            struct tm *end = getNow();
            
            if (end->tm_hour - start->tm_hour == 1) break;
            
            insertData();

            Sleep(1000);
        }

    }
    catch(SAException& e){
        cout << e.ErrText().GetMultiByteChars() << endl;
    }
    

    return 0;
}

void insertData(){
    SAConnection connection;
    SQLServer sqlObj;

    Machine machine; 
    

    connection.Connect(_TSA(sqlObj.Database.c_str()), _TSA(sqlObj.User.c_str()), 
        _TSA(sqlObj.Password.c_str()), SA_SQLServer_Client);

    SACommand cmdInsert(&connection, _TSA("INSERT INTO SA++Machine VALUES(:1, :2, :3)"));

    cmdInsert << _TSA(machine.getId().c_str()) ;

    cmdInsert.Execute();

    connection.Disconnect();

}


void createTable(){

    SAConnection connection;
    SQLServer sqlObj;

    connection.Connect(_TSA(sqlObj.Database.c_str()), _TSA(sqlObj.User.c_str()), 
        _TSA(sqlObj.Password.c_str()), SA_SQLServer_Client);

    SACommand cmdCheck(&connection, 
        _TSA("IF NOT EXISTS(SELECT * FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME=\"SA++Machine\")  \
            CREATE TABLE SA++Machine(DeviceId nvarchar(20) NOY NULL, \
            DeviceValue nvarchar(MAX) NULL, \
            UpdataTime datetime NULL);"));

    cmdCheck.Execute();

    connection.Disconnect();
    
}


void createDatabase(){
    SAConnection connection;
    SQLServer sqlObj;

    connection.Connect(_TSA(sqlObj.Database.c_str()), _TSA(sqlObj.User.c_str()), 
        _TSA(sqlObj.Password.c_str()), SA_SQLServer_Client);

    SACommand cmdCheck(&connection, 
        _TSA("IF NOT EXISTS(SELCET * FROM SYSDATABASES WHERE name=\"SA++\") CREATE Database \"SA++\""));

    cmdCheck.Execute();

    connection.Disconnect();
}

Machine generateData(){
    Machine machine;
    
    // 非確定性隨機位的來源。它允許引擎在每次執行時產生不同的隨機位元流
    random_device rndDevice;

    // 隨機引擎
    default_random_engine engine(rndDevice);
    
    //使用均勻分布模型
    uniform_int_distribution<double> randDouble(0.0, 10.0);
    uniform_int_distribution<int> randInt(0, 1000);

    /* ***************************** 產生亂數或機器所需的資料 ***************************** */
    machine.getValue()["DeviceTypeId"].assign(to_string(5));
    machine.getValue()["StatusId"].assign(to_string(0));
    machine.getValue()["ActStatus"].assign("ON");
    machine.getValue()["ActAlarmMsg"].assign("[15][1001] EMERGENCY STOP");
    machine.getValue()["ActMainProgramName"].assign("O" + to_string(randInt(engine)));
    machine.getValue()["ActMainProgramNote"].assign("");
    machine.getValue()["ActPartCount"].assign(to_string(randInt(engine)));
    machine.getValue()["Profit"].assign(to_string(randDouble(engine)));
    machine.getValue()["ProductionRequiredQty"].assign(to_string(randInt(engine)));
    machine.getValue()["ProductionRequiredQtyTotal"].assign(to_string(randInt(engine)));
    /***************************************************************************************/


    return machine;

}

struct tm* getNow(){

    time_t rawtime = time(NULL);
    tm* timeInfo = localtime(&rawtime);
    

    return timeInfo;
    
}


string tm_to_string(struct tm* tm){

    string format = "Y-%m-%d %H:%M:%S";

    char chstr[80];

    strftime(chstr, sizeof(chstr), format.c_str(), tm);

    return string(chstr);
}