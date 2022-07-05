// NOTE: 模組化

/* 
// 備忘錄: 待完成 之後須執行
*/    
#include<stdio.h>
#include<string.h>
#include<windows.h>
#include<sql.h>
#include<sqlext.h>
#include<sqltypes.h>
#include<odbcss.h>
#include<stdlib.h>
#include<time.h>
#include<assert.h>

#define SQL_ENV_FAILURE -100
#define MAXBUFFLEN 255
#define TryODBC(handle, hdtype, retcode) { \
    RETCODE rc = retcode; \
    if(rc != SQL_SUCCESS || rc != SQL_SUCCESS_WITH_INFO) \
    {\
        Diagnose(handle, hdtype, rc); \
    } \
    else { \
        fprintf(stderr, "Error: "#retcode"\n"); \
        exit(1); \
    } \
}

typedef void *Object;


SQLHENV henv = SQL_NULL_HENV; // 連線環境
SQLHDBC hdbc1 = SQL_NULL_HDBC; // 連線處理
SQLHSTMT hstmt1 = SQL_NULL_HSTMT; // 狀態處理

/* 連線字串 */
SQLCHAR ConnectionString[MAXBUFFLEN] = \
    "DRIVER={SQL Server};SERVER=140.128.109.115;UID=sa;PWD=s08490043;DATABASE=test;" ;


/* 副程式宣告 */
void Diagnose(SQLHANDLE handle, SQLSMALLINT hdType, RETCODE retcode);
void DriverConnectServer(RETCODE *retcode, SQLCHAR *connect_str);
// void PrepareStatement(SQLHSTMT *satement, SQLHDBC connection, UCHAR prepSql);
// void DisplayResults(SQLHSTMT statement,SQLSMALLINT count);
char *getToday(void * __format__);
// void AllocateData(HSTMT statement, SQLSMALLINT colCount, SQLSMALLINT *Display, Object *data);
void create_table(RETCODE *retcode);
void insert_into(RETCODE *retcode, UCHAR *sql);
void delete_data();

void Close();
// 宣告一個測試資料的結構
typedef struct Test2DBData{
    char Id[20];
    char Value[MAXBUFFLEN];
    struct Test2DBData *next;
} data_t;

UCHAR DSN[SQL_MAX_DSN_LENGTH+1] = "CSql"; // ODBC name
UCHAR UID[MAXNAME] = "test"; //the server name which you want to connect to
UCHAR AuthStr[MAXNAME] = "1234"; // password

int main(){
    /* ODBC回傳的程式碼(?? */
    RETCODE ReturnCode;

    /* 預編譯SQL語句 */
    UCHAR preprocess_sql[200];
    
    /* SQL 語句 */
    UCHAR sql[MAXBUFFLEN][1000];

    // 亂數資料
    /* 
    srand((unsigned int)time(NULL));
    for(int i = 0; i < 10; i++) {
        sprintf((char *)sql[i], "INSERT INTO CLangTest (DeviceId, DeviceValue, UpdateTime) \
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
     */
    
    /* 連線SQL Server */
    DriverConnectServer(&ReturnCode, ConnectionString);

    /* 判斷連線是否成功 */
    /* if( retcode != SQL_SUCCESS && retcode != SQL_SUCCESS_WITH_INFO)
    {
        printf("%d",retcode);
        printf("連線失敗\n請按下Enter繼續 ...... ");    fflush(stdout);
        getchar();

        exit(EXIT_FAILURE);
    }
     */
    // create_table(&retcode);


    // SQL 指令
    UCHAR SqlString[200];

    sprintf(SqlString, "INSERT INTO [dbo].[Test2](Id, Value) VALUES (\'S022%02d\', \'%s\');", 
        0, getToday(NULL));

    fprintf(stdout,"%s", SqlString);


    // 插入資料
    // insert_into(ReturnCode, SqlString);



    
    printf("操作結束。\n");
    fflush(stdout);
    getchar();


    // 關閉連線
    Close();

    return 0;
}

/* 
void AllocateData(HSTMT statement, SQLSMALLINT colCount, SQLSMALLINT *pDisplay, Object *data)
{
    SQLSMALLINT colNameLength;
    SQLLEN display_size, type_size;
    data_t *this, *last = NULL;

    *pDisplay = NULL;

    for (SQLSMALLINT i =1; i <= colCount; i++)
    {
        this = (data_t *)malloc(sizeof(data_t));
        assert(this != NULL);

        if(i == 1)  data = this;

        else    last->next = this;

        last = this;

        TRYODBC(statement,
                SQL_HANDLE_STMT,
                SQLColAttribute(statement,i,SQL_DESC_DISPLAY_SIZE,NULL,0,NULL,&display_size));

        TRYODBC(statement,
                SQL_HANDLE_STMT,
                SQLColAttribute(statement,
                    i,
                    SQL_DESC_CONCISE_TYPE,
                    NULL,
                    0,
                    NULL,
                    &type_size));
    }

} */

void insert_into(RETCODE *retcode, UCHAR sql[MAXBUFFLEN]){
    SQLSMALLINT NumResults;

    TryODBC(hstmt1, SQL_HANDLE_STMT, SQLNumResultCols(hstmt1, NumResults));

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
    *retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);

    // 直接執行 SQL
    SQLExecDirect(hstmt1, sql, (SQLINTEGER)sizeof(sql));

    printf("操作成功!\n");    fflush(stdout);

    // 釋放控制程式碼
    SQLCloseCursor(hstmt1);
    SQLFreeHandle(SQL_HANDLE_STMT, hstmt1);
}

void create_table(RETCODE *retcode){

    // 分配控制程式碼
    *retcode = SQLAllocHandle(SQL_HANDLE_STMT, hdbc1, &hstmt1);

    // 連結引數方式

    UCHAR sql[200] = "CREATE TABLE Test2( \
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


/**************************************/
/*  上方為MicroSoft官方C++程式碼寫法    */
/*  下方為網路論壇C語言寫法             */
/**************************************/
void DriverConnectServer(RETCODE *retcode, SQLCHAR *connect_str)
{   

    /****************/
    /*              */
    /* MS 官方程式碼 */
    /*              */
    /****************/
    // 分配環境
    if(SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, henv)==SQL_ERROR)
    {
        fprintf(stderr, "分配環境失敗\n");
        exit(SQL_ENV_FAILURE);
    }

    /* 註冊ODBC(??) */
    TryODBC(henv, SQL_HANDLE_ENV, 
        SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER));

    /* 建立連線 */
    TryODBC(henv, SQL_HANDLE_ENV, SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1));

    TryODBC(hdbc1, 
            SQL_HANDLE_DBC, 
            SQLDriverConnect(hdbc1, NULL, ConnectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_COMPLETE));

    fprintf(stdout, "連線成功!");    fflush(stdout);

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
    // 簡單版的連線方式
    // *retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);
    // *retcode = SQLConnect(hdbc1, DSN, 4, UID, 2, AuthStr, 13);
    //

    // 連線控制程式碼
    *retcode = SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc1);
    *retcode = SQLDriverConnect(hdbc1, NULL, ConnectionString, SQL_NTS, NULL, 0, NULL, SQL_DRIVER_NOPROMPT);
    
    */

}

/*  */
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


void Close(){
    /* 
       1. 切斷與資料來源的連線
       2. 釋放連線控制程式碼
       3. 釋放環境控制程式碼
    */
    SQLDisconnect(hdbc1);
    SQLFreeHandle(SQL_HANDLE_DBC, hdbc1);
    SQLFreeHandle(SQL_HANDLE_ENV, henv);
}

char *getToday(void * __format__){
    

    char *format = (char *)__format__;

    if(format==NULL)    format = "%Y-%m-%d %H:%M:%S";
    
    // 取得今日日期
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);

    // 格式化輸出today變數
    char today[64];
    assert(strftime(today, sizeof(today), format, tm));
    fprintf(stdout,"%s\n", today);

    return today;
}