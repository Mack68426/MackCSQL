/*  NOTE: 
*       1. 連接SQL Server
*       2. 創立資料庫，若已經有則跳過
*       3. 建立資料表，若已經有則跳過
*       4. 產生資料並插入到資料表裡
*       5. 若資料>=X筆(X=10)，則做刪除
*       6. 延遲m秒(m=1)
*       7. 重複4~6直到時間結束(T=5min)
*/

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
#include<wchar.h>

#define MAX_ARG_LENGTH 0x10001
#define SQL_ENV_FAILURE -100
#define MAXBUFFLEN 256



// Exception Handle for ODBC functions
#define TryODBC(handle, hdtype, retcode) { \
    SQLRETURN rc = retcode; \
    if(rc != SQL_SUCCESS) \
    { \
        Diagnose(handle, hdtype, rc); \
    } \
    if(rc == SQL_ERROR){ \
        fprintf(stderr, "Error in "#retcode"\nState: %d\n", rc); \
        if(hstmt1)  SQLFreeHandle(SQL_HANDLE_STMT, hstmt1); \
        if (hdbc1) { SQLDisconnect(hdbc1); SQLFreeHandle(SQL_HANDLE_DBC, hdbc1); } \
        if (henv)   SQLFreeHandle(SQL_HANDLE_ENV, henv); \
        exit(1); \
    } \
}


typedef struct MachineValue{
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
} machval_t;




/* 副程式宣告 */
void Diagnose(SQLHANDLE handle, SQLSMALLINT hdType, RETCODE retcode);
void Connect(char *);
void Prepare(SQLHSTMT *satement, SQLHDBC connection, SQLCHAR prepSql);
char GenerateData();
void Close();
// void DisplayResults(SQLHSTMT statement,SQLSMALLINT count);
char *getToday();
char *Format(char *format, ...); 
//void AllocateData(SQLSMALLINT colCount, machine_t **data);


// 資料庫操作
void create_database(char *dbName);
void create_table(char *tableName);
void insert_into(char *tableName);
void delete_data();

// 其他自訂函式
int asprintf(char **buffer, const char *format, ...);
int vasprintf(char **buffer, const char *format, va_list ap);

/* ***************************** 全域變數區 ***************************** */
SQLHENV henv = SQL_NULL_HENV; // 連線環境
SQLHDBC hdbc1 = SQL_NULL_HDBC; // 連線處理
SQLHSTMT hstmt1 = SQL_NULL_HSTMT; // 狀態處理

/* 
SQLCHAR *DSN = "CSql";
SQLCHAR *host = "140.128.109.115";
SQLCHAR *server = "sa";
SQLCHAR *password = "s08490043";
SQLCHAR *Database = "test";
 */

// 連線字串格式 
// SQLCHAR *ConnStrFmt = 
    // "SERVER=%s;UID=%s;PWD=%s;TrustServerCertificate=true;DATABASE=%s;";

/*************************************************************************/


int main(){
    
    srand((unsigned int)time(NULL));
    
    
    // 建立一個資料庫
    create_database("OdbcDB");
    
    // 建立資料表
    // create_table("Proceesor");

    // 插入資料
    // insert_into();

    printf("操作結束。\n..... ");
    fflush(stdout);
    getchar();
    

    // 關閉連線
    Close();

    return 0;
}

// 未完成
/* void AllocateData(SQLSMALLINT colCount, machine_t **data)
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

} */

void insert_into(char *tableName){

    // 分配控制程式碼
    TryODBC(hstmt1, SQL_HANDLE_STMT, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

    Connect(Format("Server=%s;UID=%s;\
            PWD=%s;TrustServerCertificate=true;DATABASE=%s;",
            "140.128.109.115", "sa", "s08490043", "OdbcDB"));
    
    SQLSMALLINT NumResults;

    char *checkFmt = "SELECT COUNT(*) FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME=\"%s\";";

    char *sql = Format(checkFmt,tableName);
    
    SQLRETURN state;
    
    
    TryODBC(hstmt1, SQL_HANDLE_STMT, SQLExecDirect(hstmt1, (SQLCHAR *)sql, (SQLINTEGER)strlen(sql)))


    // 直接執行 SQL
    SQLExecDirect(hstmt1, 
        Format("INSERT INTO %s VALUES(\"%s\", %s, %s);", tableName), 255);

    printf("操作成功!\n");    fflush(stdout);

    Close();

    // 釋放控制程式碼
    SQLCloseCursor(hstmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
}

void create_table(char *tableName){

    // 分配控制程式碼
    TryODBC(&hstmt1, SQL_HANDLE_STMT, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

    // 連接server
    Connect(Format("Server=%s;UID=%s;PWD=%s;TrustServerCertificate=true;DATABASE=\"%s\";",
            "140.128.109.115:1433", "sa", "s08490043", "OdbcDB"));

    

    char sql_fmt[] = "CREATE TABLE %s \
    ( \
        DeviceId nvarchar(20) NOT NULL, \
        DeviceValue nvarchar(MAX) NULL, \
        UpdateTime datetime NULL \
    );";

    /* 直接執行SQL語句 */
    SQLExecDirect(hstmt1, Format(sql_fmt, tableName), 200);
    
    printf("操作成功!!\n請按下Enter繼續....."); fflush(stdout);
    getchar();

    Close();

    // 釋放控制程式碼
    SQLCloseCursor(hstmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);

}

void create_database(char *dbName){

    // TryODBC(hstmt1, SQL_HANDLE_STMT, SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1));

    Connect(Format("Server=%s;UID=%s;PWD=%s;\
        TrustServerCertificate=true;", "140.128.109.115", "sa", "s08490043"));

    

    char *sql = Format("SELECT COUNT(*) FROM SYSDATABASES WHERE name=\"%u\"", dbName);

    TryODBC(hstmt1, SQL_HANDLE_STMT, SQLExecDirect(hstmt1, sql, (SQLINTEGER)strlen(sql)))
    

    SQLExecDirect(hstmt1, Format("CREATE DATABASE %s", dbName), 200);

    printf("\n操作成功!!\n請按下Enter繼續....."); fflush(stdout);
    getchar();

    Close();

    // 釋放控制程式碼
    SQLCloseCursor(hstmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);

}


/*******************************************/
/*      上方為MicroSoft官方C++文件寫法       */
/*          下方為網路論壇C語言寫法          */
/*******************************************/

// 行282: SQLDriverConnect會報錯需修正
void Connect(char *sql)
{   
    /**************************/
    /*                        */
    /*      MS 官方程式碼      */
    /*                        */
    /**************************/

    // 分配環境
    if(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv)==SQL_ERROR)
    {
        fprintf(stderr, "分配環境失敗\n");
        exit(SQL_ENV_FAILURE);
    }

    /* 連接資料來源 */
    TryODBC(henv, SQL_HANDLE_ENV, 
        SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER));

    /* 分配連線資源 */
    TryODBC(henv, SQL_HANDLE_ENV, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));
    

    SQLCHAR outStr[MAXBUFFLEN];
    SQLSMALLINT outStrLen;

    // 連線Server
    TryODBC(hdbc1, 
        SQL_HANDLE_DBC, 
        SQLDriverConnect(hdbc1, GetDesktopWindow(),
            (SQLCHAR *)sql,
            SQL_NTS, 
            NULL, 0, NULL, SQL_DRIVER_COMPLETE));

    fprintf(stdout,"連線成功!\n 連線字串: \n");


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



/* 關閉連線 */
void Close(){
    /* 1. 切斷與資料來源的連線  */
    /* 2. 釋放連線控制程式碼    */
    /* 3. 釋放環境控制程式碼    */
    SQLDisconnect(hdbc1);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

/* 診斷ODBC函式狀態 */
void Diagnose(SQLHANDLE handle, SQLSMALLINT hdType, RETCODE retcode)
{
    SQLSMALLINT records = 0;
    SQLINTEGER Error;
    int i = 0;
    WCHAR message[1024];
    WCHAR state[SQL_SQLSTATE_SIZE + 1];

    if(retcode == SQL_INVALID_HANDLE){
        fprintf(stderr, "Invalid handle!\n");
        return ;
    }

    SQLRETURN diag = SQLGetDiagRecW(hdType, handle, ++records, state, &Error, message, 
        (SQLSMALLINT)(sizeof(message) / sizeof(WCHAR)), (SQLSMALLINT *)NULL);
        
    
    while( diag == SQL_SUCCESS){
        if (i>10)   break;

        if(wcsncmp(state, L"01004", 5)){
            fwprintf(stderr, L"[%5.5s] %s (%d)\n", state, message, Error);
        }

        i++;
    }
}

// 待完成
char GenerateData(){
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

char *Format(char *format, ...)
{
    if (format == NULL) return "";

    else{
        va_list argsList;
        va_start(argsList, format);
        char *str=NULL;
        
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