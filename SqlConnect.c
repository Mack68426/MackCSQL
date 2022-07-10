// NOTE: 

/* 備忘錄:
*   + 模組化
*       將所需功能寫成副程式
*       將功能相似的副程式結合成一個函式庫(OPT)
*
*   + 註解
*       為每項副程式、巨集、以及宣告加上註解以便日後閱讀
*/
#include<stdio.h>
#include<string.h>
#include<windows.h>
#include<sql.h>
#include<sqlext.h>
#include<sqltypes.h>
#include<sqlucode.h>
#include<odbcinst.h>
#include<odbcss.h>
#include<stdlib.h>
#include<time.h>
#include<assert.h>

#define MAX_ARG_LENGTH 0x10001
#define SQL_ENV_FAILURE -100
#define MAXBUFFLEN 256

// Exception Handle for ODBC functions
#define TryODBC(handle, hdtype, retcode) { \
    RETCODE rc = retcode; \
    if(rc == SQL_SUCCESS || rc == SQL_SUCCESS_WITH_INFO) \
    {\
        Diagnose(handle, hdtype, rc); \
    } \
    else { \
        fprintf(stderr, "Error: %s\n",retcode); \
        exit(1); \
    } \
}

typedef void *Object;

typedef struct Machine{
    int DeviceId;
    int StatusId;
    SQLCHAR ActStatus[10];
    SQLCHAR *ActAlarmMsg;
    SQLCHAR *ActMainProgramName;
    SQLCHAR *ActMainProgramNote;
    int ActPartCount;
    double Profit;
    int ProductionRequiredQty;
    int ProductionRequiredQtyTotal;
    struct Machine *Next;
} machine_t;


/* 副程式宣告 */
void Diagnose(SQLHANDLE handle, SQLSMALLINT hdType, RETCODE retcode);
void Connect(RETCODE *retcode, SQLCHAR *connect_str);
void Prepare(SQLHSTMT *satement, SQLHDBC connection, SQLCHAR prepSql);
char *GenerateData();
void Close();
// void DisplayResults(SQLHSTMT statement,SQLSMALLINT count);
char *getToday();
char *Format(char *format, ...); 
//void AllocateData(SQLSMALLINT colCount, machine_t **data);


// 資料庫操作
void create_database(SQLCHAR *dbName);
void create_table(RETCODE *retcode);
void insert_into(RETCODE *retcode, SQLCHAR sql[MAXBUFFLEN]);
void delete_data();

// 其他函式
int asprintf(char **buffer, const char *format, ...);
int vasprintf(char **buffer, const char *format, va_list ap);

/* ***************************** 全域變數區 ***************************** */
SQLHENV henv = SQL_NULL_HENV; // 連線環境
SQLHDBC hdbc1 = SQL_NULL_HDBC; // 連線處理
SQLHSTMT hstmt1 = SQL_NULL_HSTMT; // 狀態處理

SQLCHAR *DSN = "CSql";
SQLCHAR *host = "140.128.109.115";
SQLCHAR *User = "sa";
SQLCHAR *password = "s08490043";
SQLCHAR *Database = "test";

/* 連線字串格式 */
SQLCHAR *ConnStrFmt = 
    "SERVER=%s;UID=%s;PWD=%s;TrustServerCertificate=true;DATABASE=%s;";


/*************************************************************************/


int main(){
    
    /* ODBC回傳的程式碼(?? */
    RETCODE ReturnCode;

    srand((unsigned int)time(NULL));
    
    /* 連線SQL Server */
    Connect(&ReturnCode, ConnectionString(ConnStrFmt));

    // 建立一個資料庫
    // create_database("CSQLDB");
    
    // 建立資料表
    create_table(&ReturnCode);

    // 插入資料
    // insert_into(ReturnCode, SqlString);

    printf("操作結束。\n..... ");
    fflush(stdout);
    getchar();
    

    // 關閉連線
    Close();

    return 0;
}

// 未完成
/*
void AllocateData(SQLSMALLINT colCount, machine_t **data)
{
    machine_t *this, *last = NULL;

    this = (machine_t *)malloc(sizeof(machine_t));

    assert(this != NULL);

    for (int i = 1; i <= colCount; i++)
    {
        if (i == 1) *data = this;
        else last->Next = this;

        last = this;
        

    }

}
*/
void insert_into(RETCODE *retcode, SQLCHAR sql[MAXBUFFLEN]){
    
    SQLSMALLINT NumResults;

    TryODBC(hstmt1, SQL_HANDLE_STMT, SQLNumResultCols(hstmt1, &NumResults));

    // if(NumResults>0)    DisplayResults(hstmt1,NumResults);

    

    SQLLEN rows_count;

    TryODBC(hstmt1, SQL_HANDLE_STMT, SQLRowCount(hstmt1, &rows_count));

    if(rows_count>=0) {

        fprintf(stdout, "%lld row%s affected\n請按下Enter繼續 .....", 
            rows_count, rows_count==1?"":"s");
        
        fflush(stdout);

        getchar();
    }

    // 分配控制程式碼
    TryODBC(hstmt1, SQL_HANDLE_STMT, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

    // 直接執行 SQL
    SQLExecDirect(hstmt1, sql, 255);

    printf("操作成功!\n");    fflush(stdout);

    // 釋放控制程式碼
    SQLCloseCursor(hstmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
}

void create_table(RETCODE *retcode){

    // 分配控制程式碼
    *retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);

    // 連結引數方式

    SQLCHAR sql[200] = "CREATE TABLE Computer( \
        Id VARCHAR(20) NOT NULL, \
        Value NVARCHAR(MAX) NULL, \
    );";
    /* 直接執行SQL語句 */
    SQLExecDirect(hstmt1, sql, 200);
    
    printf("操作成功!!\n請按下Enter繼續....."); fflush(stdout);
    getchar();

    // 釋放控制程式碼
    SQLCloseCursor(hstmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);

}

void create_database(SQLCHAR *dbName){

    SQLCHAR *sql = (SQLCHAR *)Format("CREATE DATABASE %s", dbName);

    TryODBC(&hstmt1, SQL_HANDLE_STMT, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

    SQLExecDirect(&hstmt1, sql, 200);

    printf("\n操作成功!!\n請按下Enter繼續....."); fflush(stdout);
    getchar();

    // 釋放控制程式碼
    SQLCloseCursor(hstmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
}


/**************************************/
/*  上方為MicroSoft官方C++程式碼寫法    */
/*  下方為網路論壇C語言寫法             */
/**************************************/
void Connect(RETCODE *retcode, SQLCHAR *connect_str)
{   

    char outStr[MAXBUFFLEN];
    /****************/
    /*              */
    /* MS 官方程式碼 */
    /*              */
    /****************/
    // 分配環境
    if(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv)==SQL_ERROR)
    {
        fprintf(stderr, "分配環境失敗\n");
        exit(SQL_ENV_FAILURE);
    }

    /* 連接資料來源 */
    TryODBC(henv, SQL_HANDLE_ENV, 
        SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER));

    /* 建立連線 */
    TryODBC(henv, SQL_HANDLE_ENV, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

    // 連線Driver
    TryODBC(hdbc1, 
            SQL_HANDLE_DBC, 
            SQLDriverConnect(hdbc1, GetDesktopWindow(), 
                Format(ConnStrFmt,*host, User, password,Database), 
                SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE));

    fprintf(stdout, "連線成功!\n 連線字串: %s", outStr);    fflush(stdout);


    TryODBC(hdbc1,
            SQL_HANDLE_DBC,
            SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));    



    /************************/
    /*                      */
    /*    論壇C語言程式碼    */
    /*                      */
    /************************/

    /* 

    // 連線資料來源
    *retcode = SQLAllocHandle(SQL_HANDLE_ENV, NULL, &henv);
    *retcode = SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
    
    /************************/
    /*                      */
    /*    簡單版的連線方式   */
    /*                      */
    /************************/
    // *retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);
    // *retcode = SQLConnect(hdbc1, DSN, 4, UID, 2, AuthStr, 13);
    //

    // 連線控制程式碼
    // *retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);
    // *retcode = SQLDriverConnect(hdbc1, NULL, ConnectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    

}

/* 診斷ODBC函式狀態 */
void Diagnose(SQLHANDLE handle, SQLSMALLINT hdType, RETCODE retcode)
{
    SQLSMALLINT records = 0;
    SQLINTEGER Error;

    CHAR message[1024];
    CHAR state[SQL_SQLSTATE_SIZE + 1];

    if(retcode == SQL_INVALID_HANDLE){
        fprintf(stderr, "Invalid handle!\n");
        return ;
    }

    SQLRETURN diag = SQLGetDiagRec(hdType,handle, ++records, state, &Error, message, 
        (SQLSMALLINT)(sizeof(message) / sizeof(CHAR)), (SQLSMALLINT *)NULL);
    
    while( diag == SQL_SUCCESS){
        if(strncmp(state,"t0001",5)){
            fprintf(stderr, "[%s] %s (%d)\n", state, message, Error);
        }
    }
}

/* 關閉連線 */
void Close(){
    /* 1. 切斷與資料來源的連線  */
    /* 2. 釋放連線控制程式碼    */
    /* 3. 釋放環境控制程式碼    */
    SQLDisconnect(hdbc1);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

char *getToday(){
    
    // 取得今日日期
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    // 格式化輸出today變數
    char *today = malloc(sizeof(char) * 64);
    assert(strftime(today, sizeof(today), "%Y-%m-%d %H:%M:%S", tm));
    fprintf(stdout,"%s\n", today);

    return today;
}

char *GenerateData(){
    // 亂數資料
    /* 
    char *m;
    
    for(int i = 0; i < 10; i++) {
        sprintf(data, 
            "INSERT INTO CLangTest (DeviceId, DeviceValue, UpdateTime) \
            VALUES(\"MACHINE%d\", \"{   \"DeviceTypeId\":%u,\
                                \"StatusId\":%u,\
                                \"ActStatus\":\"ON\",\
                                \"ActAlarmMsg\":\"[15][1001] EMERGENCY STOP\",\
                                \"ActMainProgramName\":\"O602\",\
                                \"ActMainProgramNote\":\"\",\
                                \"ActPartCount\":%u,\
                                \"Profit\":%lf,\
                                \"ProductionRequiredQty\":%u,\
                                \"ProductionRequiredQtyTotal\":%u}\",%s);",
        i+1, (unsigned int)rand()%11+1, (unsigned int)rand()%3, (unsigned int)rand()%1000+1,
        rand()%101+1.0, (unsigned int)rand()%11+1, (unsigned int)rand()%11+1, today);
    }

    return m;
     */
}

char *Format(char *format, ...)
{
    if (format == NULL) return NULL;

    else{
        va_list argsList;
        va_start(argsList, format);
        char *str;
        
        asprintf(&str, format, argsList);

        va_end(argsList);

        return str;
    }
}

int vasprintf(char **buffer, const char *format, va_list ap) {

    int len = vsnprintf(NULL, 0, format, ap);
    char *str = malloc(sizeof(char) * (len + 1));

    assert(str != NULL);
    vsnprintf(str, len + 1, format, ap);
    *buffer = str;

    return len;
}


int asprintf(char **buffer, const char *format, ...) {
    va_list ap;
    va_start(ap, format);
    int ret = vasprintf(buffer, format, ap);
    
    va_end(ap);

    return ret;
}