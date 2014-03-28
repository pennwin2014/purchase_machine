/******************************************************************************
* Copyright SupWisdom 2012 all right reserved
* Module: 参数数据库操作接口
* File: syspara.c
* Auth: Cheng Tang<cheng.tang@supwisdom.com>
******************************************************************************/

#include <limits.h>
#include <assert.h>

#include "p16log.h"
#include "syspara.h"
#include "utils.h"
#include "decard/sqlite3.h"

static char cfg_db_path[PATH_MAX + 1] = {0};
static sqlite3* cfg_db = NULL;
////////////////////////////////////////////////////////////////////////////////
typedef struct
{
    sqlite3_stmt* stmt;
} sqlite_result_set_t;


static void sql_free_resultset(sqlite_result_set_t* resultset)
{
    if (resultset->stmt)
    {
        sqlite3_finalize(resultset->stmt);
    }
    free(resultset);
}

static int sql_execute(sqlite3* db, const char* sql, sqlite_result_set_t** rs)
{
    sqlite_result_set_t* result;
    result = (sqlite_result_set_t*)malloc(sizeof(sqlite_result_set_t));
    result->stmt = NULL;
    const char* ztail = 0;
    int rc = sqlite3_prepare_v2(db, sql, strlen(sql), &(result->stmt), &ztail);
    if (rc != SQLITE_OK)
    {
        LOG((LOG_ERROR, "准备sql语句错误，rc=%d,msg=%s", rc, sqlite3_errmsg(db)));
        sql_free_resultset(result);
        return -1;
    }
    *rs = result;
    return 0;
}
static int sql_fetch_resultset(sqlite_result_set_t* resultset)
{
    int r = sqlite3_step(resultset->stmt);
    if (r == SQLITE_ROW)
        return 0;
    if (r == SQLITE_DONE)
        return 1;
    return -1;
}

////////////////////////////////////////////////////////////////////////////////

static int sql_exec_stmt(sqlite3* db, const char* sql)
{
    int rc;
    char* emsg = NULL;
    rc = sqlite3_exec(db, sql, NULL, NULL, &emsg);
    if (rc != SQLITE_OK)
    {
        LOG((LOG_ERROR, "执行SQL语句失败,rc=%d,msg=%s", rc, emsg));
    }
    sqlite3_free(emsg);
    return (rc == SQLITE_OK) ? 0 : rc;
}
#if 0
static int do_qry_into_callback(void* into, int argc, char** argv, char** azColName)
{
    assert(argc == 1);
    //LOG((LOG_DEBUG,"查询: count: %d",argc));
    strcpy((char*)into, argv[0]);
    return 0;
}
#endif
////////////////////////////////////////////////////////////////////////////////
static int8 do_initial_syspara()
{
    /*
    char *init_para_sql[] =
    {
      "INSERT INTO SYSPARA(PARANAME,PARAVAL,PARAFLAG)VALUES('sys.password','51719228',1)",
      "INSERT INTO SYSPARA(PARANAME,PARAVAL,PARAFLAG)VALUES('sys.remoteip','192.168.1.200',1)",
      "INSERT INTO SYSPARA(PARANAME,PARAVAL,PARAFLAG)VALUES('sys.remoteport','8080',1)",
      "INSERT INTO SYSPARA(PARANAME,PARAVAL,PARAFLAG)VALUES('sys.updatesvrip','192.168.1.44',1)",
      "INSERT INTO SYSPARA(PARANAME,PARAVAL,PARAFLAG)VALUES('dev.devphyid','',1)",
      "INSERT INTO SYSPARA(PARANAME,PARAVAL,PARAFLAG)VALUES('dev.ip','192.168.16.239',1)",
      "INSERT INTO SYSPARA(PARANAME,PARAVAL,PARAFLAG)VALUES('dev.netmask','255.255.255.0',1)",
      "INSERT INTO SYSPARA(PARANAME,PARAVAL,PARAFLAG)VALUES('dev.gateway','192.168.16.1',1)",
      NULL,
    };
    */
    struct
    {
        const char* paraname;
        const char* paraval;
    } init_para[] =
    {
        {"sys.password", "51719228"},
        {"sys.remoteip", "192.168.1.200"},
        {"sys.remoteport", "8080"},
        {"sys.updatesvrip", "192.168.1.44"},
        {"dev.devphyid", ""},
        {"dev.ip", "192.168.16.239"},
        {"dev.netmask", "255.255.255.0"},
        {"dev.gateway", "192.168.16.1"},
        {"sys.appid", "500000"},
        {"sys.appsecret", "dafd8bd1ca659d8ded20bbacc123"},
        {"svc.remotename", "192.168.1.200"},
        {"svc.remoteport", "8080"},
        {"svc.remoteurl", "/yktapi/services"},
        {"sys.printerpages", "1"},
        {"sys.cfgverno", "0"},
        {"sys.workmode", "0"},
        {"sys.heartbeatminutes", "3"},
        {NULL, NULL},
    };
    int rc;
    sqlite_result_set_t* rs = NULL;
    rc = sql_execute(cfg_db, "INSERT INTO SYSPARA(PARANAME,PARAVAL,PARAFLAG)VALUES(?,?,?);",
                     &rs);
    if (rc)
    {
        return -1;
    }
    int i;
    char temp[257];
    for (i = 0;; ++i)
    {
        if (init_para[i].paraname == NULL)
        {
            rc = 0;
            break;
        }

        if (SAFE_GET_CONFIG(init_para[i].paraname, temp) == 0)
            continue;
        LOG((LOG_DEBUG, "初始化参数%s = %s", init_para[i].paraname, init_para[i].paraval));
        sqlite3_bind_text(rs->stmt, 1, init_para[i].paraname, -1, SQLITE_STATIC);
        sqlite3_bind_text(rs->stmt, 2, init_para[i].paraval, -1, SQLITE_STATIC);
        sqlite3_bind_int(rs->stmt, 3, 1);
        rc = sqlite3_step(rs->stmt);
        if (rc != SQLITE_DONE)
        {
            LOG((LOG_ERROR, "初始化系统参数错误"));
            rc = -1;
            break;
        }
        sqlite3_reset(rs->stmt);
    }
    sql_free_resultset(rs);
    return rc;
}
int8 syspara_reset()
{
    int ret;
    ret = sql_exec_stmt(cfg_db, "DELETE FROM SYSPARA");
    if (ret)
    {
        return -1;
    }
    return  do_initial_syspara();
}

int8 check_config_db()
{
    if (strlen(cfg_db_path) == 0)
    {
        join_path(p16pos.base_dir, "../config/p16cfg.db", cfg_db_path);
        LOG((LOG_DEBUG, "配置文件路径,%s", cfg_db_path));
    }
    int ret;
    ret = sqlite3_open(cfg_db_path, &cfg_db);
    if (ret != SQLITE_OK)
    {
        error_exit(1, "不能创建/打开配置数据");
    }

    ret = sql_exec_stmt(cfg_db, "SELECT COUNT(*) FROM SYSPARA");
    if (ret)
    {
        // 创建数据库
        ret = sql_exec_stmt(cfg_db, "CREATE TABLE SYSPARA( \
			PARANAME VARCHAR(50) NOT NULL,\
			PARAVAL VARCHAR(255) ,\
			PARAFLAG INTEGER,\
			PRIMARY KEY (PARANAME))");
        if (ret)
        {
            LOG((LOG_ERROR, "创建参数表失败"));
            return -1;
        }

    }
    ret = do_initial_syspara();
    if (ret)
        return -1;

    ret = sql_exec_stmt(cfg_db, "SELECT COUNT(*) FROM OPERSTAT");
    if (ret)
    {
        // 创建数据库
        ret = sql_exec_stmt(cfg_db, "CREATE TABLE OPERSTAT( \
			OPERDATE VARCHAR(8) NOT NULL,\
			BATCHNO INTEGER NOT NULL,\
			OPERID INTEGER NOT NULL,\
			OPERCODE VARCHAR(10),\
			SUCCCNT INTEGER NOT NULL DEFAULT 0,\
			SUCCAMT INTEGER NOT NULL DEFAULT 0,\
			ERRCNT INTEGER NOT NULL DEFAULT 0,\
			ERRAMT INTEGER NOT NULL DEFAULT 0,\
			PRIMARY KEY (OPERDATE,BATCHNO,OPERID))");
        if (ret)
        {
            LOG((LOG_ERROR, "创建统计表失败"));
            return -1;
        }
    }

    ret = sql_exec_stmt(cfg_db, "SELECT COUNT(*) FROM TRANSDTL");
    if (ret)
    {
        // 创建数据库
        /*
         * TRANSFLAG - 表示写卡状态，0 表示未发起写卡动作，1 表示写卡成功，
         *             2 表示写卡失败，3 表示中途拔卡,4 表示后台交易返回成功，5 表示后台交易返回失败
         * CONFIRM - 表示确认标志，0 表示未发起确认，1 表示已发起确认
         */
        ret = sql_exec_stmt(cfg_db, "CREATE TABLE TRANSDTL( \
			TRANSDATE VARCHAR(8) NOT NULL,\
			TRANSTIME VARCHAR(6) NOT NULL,\
			CARDNO INTEGER NOT NULL,\
			DEVPHYID VARCHAR(16) NOT NULL,\
			DEVSEQNO INTEGER NOT NULL,\
			OPERID INTEGER NOT NULL,\
			CARDPHYID VARCHAR(16) NULL,\
			CARDBEFBAL INTEGER,\
			DPSCNT INTEGER,\
			PAYCNT INTEGER,\
			AMOUNT INTEGER,\
			DPSAMT INTEGER,\
			TERMNO VARCHAR(12),\
			TAC VARCHAR(8),\
			TRANSFLAG INTEGER NOT NULL DEFAULT 0,\
			CONFIRM INTEGER NOT NULL DEFAULT 0,\
			AUTHCODE INTEGER,\
			BATCHNO INTEGER,\
			TERMID INTEGER,\
			REFNO VARCHAR(20),\
			PRIMARY KEY (TRANSDATE,TRANSTIME,CARDNO,OPERID,DEVPHYID,DEVSEQNO))");
        if (ret)
        {
            LOG((LOG_ERROR, "创建流水表失败"));
            return -1;
        }
    }

    ret = sql_exec_stmt(cfg_db, "SELECT COUNT(*) FROM DPSFEERATE");
    if (ret)
    {
        // 创建数据库
        /*
         * feetype 卡收费类别, 收费类别不存在就不允许充值
         * feerate 费率，5 表示收取 5% 手续费， 0 表示不收取手续费
         * feeflag 费率类型， 1 表示按比例收取， 2 表示固定金额，单位 分
         */
        ret = sql_exec_stmt(cfg_db, "CREATE TABLE DPSFEERATE(\
              FEETYPE INTEGER NOT NULL, \
      FEERATE INTEGER NOT NULL DEFAULT 0, \
      FEEFLAG INTEGER NOT NULL DEFAULT 1, \
      PRIMARY KEY (FEETYPE))");
        if (ret)
        {
            LOG((LOG_ERROR, "创建充值费率表失败"));
            return -1;
        }
    }

    ret = sql_exec_stmt(cfg_db, "SELECT COUNT(*) FROM ALLOWANCEDETAIL");
    if (ret)
    {
        ret = sql_exec_stmt(cfg_db, "CREATE TABLE ALLOWANCEDETAIL( \
              TRANSDATE VARCHAR(8) NOT NULL,\
              OPERID INTEGER NOT NULL,\
              DEVPHYID VARCHAR(16) NOT NULL,\
              DEVSEQNO INTEGER NOT NULL,\
              SUBSIDYNO INTEGER NOT NULL,\
              AMOUNT INTEGER NOT NULL,\
              PRIMARY KEY(SUBSIDYNO))");
        if (ret)
        {
            LOG((LOG_ERROR, "创建补助明细表失败"));
            return -1;
        }
    }
    return 0;
}
void close_config_db()
{
    if (NULL != cfg_db)
    {
        sqlite3_close(cfg_db);
        cfg_db = NULL;
    }
}
int8 update_config(const char* paraname, char* paraval)
{
    char sql[1024];
    sprintf(sql, "UPDATE SYSPARA SET PARAVAL='%s' WHERE PARANAME='%s'", paraval, paraname);
    int ret;
    ret = sql_exec_stmt(cfg_db, sql);
    if (ret)
    {
        LOG((LOG_ERROR, "更新参数失败"));
        return -1;
    }
    return 0;
}
int8 update_config_int(const char* paraname, int paraval)
{
    char temp[24];
    sprintf(temp, "%d", paraval);
    return update_config(paraname, temp);
}
int8 get_config(const char* paraname, char* paraval , int maxlen)
{
    // char temp[257] = {0};
    char sql[512];
    sqlite_result_set_t* rs = NULL;
    int rc;
    sprintf(sql, "SELECT PARAVAL FROM SYSPARA WHERE PARANAME='%s'", paraname);
    //LOG( ( LOG_DEBUG, sql ) );
    if (sql_execute(cfg_db, sql, &rs))
        return -1;

    if (!sql_fetch_resultset(rs))
    {
        const char* val = sqlite3_column_text(rs->stmt, 0);
        memcpy(paraval, val, maxlen);
        rc = 0;
    }
    else
    {
        LOG((LOG_ERROR, "参数 %s 无记录", paraname));
        rc = -1;
    }
    sql_free_resultset(rs);
    return rc;
}
int8 get_config_int(const char* paraname, int* paraval)
{
    char temp[257] = {0};
    if (get_config(paraname, temp , sizeof(temp) - 1))
    {
        return -1;
    }
    *paraval = atoi(temp);
    return 0;
}

int8 trans_add_record(p16_transdtl_t* record)
{
    char sql[1024];
    strcpy(sql, "INSERT INTO TRANSDTL(TRANSDATE,TRANSTIME,CARDNO,DEVPHYID,\
					DEVSEQNO,OPERID,CARDPHYID,CARDBEFBAL,DPSCNT,PAYCNT,AMOUNT,DPSAMT,TERMNO,\
					TAC,TRANSFLAG,CONFIRM,AUTHCODE,BATCHNO,TERMID,REFNO)VALUES(?,?,?,?,?,?,?,?,?,\
					?,?,?,?,?,?,?,?,?,?,?);");

    int rc;
    sqlite_result_set_t* rs = NULL;
    rc = sql_execute(cfg_db, sql, &rs);
    if (rc)
    {
        return -1;
    }
    sqlite3_bind_text(rs->stmt, 1, record->transdate, -1, SQLITE_STATIC);
    sqlite3_bind_text(rs->stmt, 2, record->transtime, -1, SQLITE_STATIC);
    sqlite3_bind_int(rs->stmt, 3, record->cardno);
    sqlite3_bind_text(rs->stmt, 4, record->devphyid, -1, SQLITE_STATIC);
    sqlite3_bind_int(rs->stmt, 5, record->devseqno);
    sqlite3_bind_int(rs->stmt, 6, record->operid);
    sqlite3_bind_text(rs->stmt, 7, record->cardphyid, -1, SQLITE_STATIC);
    sqlite3_bind_int(rs->stmt, 8, record->cardbefbal);
    sqlite3_bind_int(rs->stmt, 9, record->dpscnt);
    sqlite3_bind_int(rs->stmt, 10, record->paycnt);
    sqlite3_bind_int(rs->stmt, 11, record->amount);
    sqlite3_bind_int(rs->stmt, 12, record->dpsamt);
    sqlite3_bind_text(rs->stmt, 13, record->termno, -1, SQLITE_STATIC);
    sqlite3_bind_text(rs->stmt, 14, record->tac, -1, SQLITE_STATIC);
    sqlite3_bind_int(rs->stmt, 15, record->transflag);
    sqlite3_bind_int(rs->stmt, 16, record->confirm);
    sqlite3_bind_int(rs->stmt, 17, record->authcode);
    sqlite3_bind_int(rs->stmt, 18, record->batchno);
    sqlite3_bind_int(rs->stmt, 19, record->termid);
    sqlite3_bind_text(rs->stmt, 20, record->refno, -1, SQLITE_STATIC);
    rc = sqlite3_step(rs->stmt);
    sql_free_resultset(rs);
    if (rc != SQLITE_DONE)
    {
        LOG((LOG_ERROR, "增加数据错误。rc=%d,msg=[%s]", rc , sqlite3_errmsg(cfg_db)));
        return -1;
    }
    return 0;
}
int8 trans_get_last_record(p16_transdtl_t* record)
{
    char sql[1024];
    strcpy(sql, "SELECT TRANSDATE,TRANSTIME,CARDNO,DEVPHYID,\
					DEVSEQNO,OPERID,CARDPHYID,CARDBEFBAL,DPSCNT,PAYCNT,AMOUNT,DPSAMT,TERMNO,\
					TAC,TRANSFLAG,CONFIRM,AUTHCODE,BATCHNO,TERMID,REFNO FROM TRANSDTL WHERE \
					CONFIRM=0 ORDER BY TRANSDATE ,TRANSTIME");
    sqlite_result_set_t* rs = NULL;
    int rc;
    if (sql_execute(cfg_db, sql, &rs))
        return -1;

    if (!sql_fetch_resultset(rs))
    {
        SAFE_STR_CPY(record->transdate, sqlite3_column_text(rs->stmt, 0));
        SAFE_STR_CPY(record->transtime, sqlite3_column_text(rs->stmt, 1));
        record->cardno = sqlite3_column_int(rs->stmt, 2);
        SAFE_STR_CPY(record->devphyid, sqlite3_column_text(rs->stmt, 3));
        record->devseqno = sqlite3_column_int(rs->stmt, 4);
        record->operid = sqlite3_column_int(rs->stmt, 5);
        SAFE_STR_CPY(record->cardphyid, sqlite3_column_text(rs->stmt, 6));
        record->cardbefbal = sqlite3_column_int(rs->stmt, 7);
        record->dpscnt = sqlite3_column_int(rs->stmt, 8);
        record->paycnt = sqlite3_column_int(rs->stmt, 9);
        record->amount = sqlite3_column_int(rs->stmt, 10);
        record->dpsamt = sqlite3_column_int(rs->stmt, 11);
        SAFE_STR_CPY(record->termno, sqlite3_column_text(rs->stmt, 12));
        SAFE_STR_CPY(record->tac, sqlite3_column_text(rs->stmt, 13));
        record->transflag = sqlite3_column_int(rs->stmt, 14);
        record->confirm = sqlite3_column_int(rs->stmt, 15);
        record->authcode = sqlite3_column_int(rs->stmt, 16);
        record->batchno = sqlite3_column_int(rs->stmt, 17);
        record->termid = sqlite3_column_int(rs->stmt, 18);
        SAFE_STR_CPY(record->refno, sqlite3_column_text(rs->stmt, 19));
        rc = 1;
    }
    else
    {
        LOG((LOG_ERROR, "无流水记录"));
        rc = 0;
    }
    sql_free_resultset(rs);
    return rc;
}
int8 trans_update_record(p16_transdtl_t* record)
{
    char sql[1024];
    strcpy(sql, "UPDATE TRANSDTL SET CARDPHYID=?,CARDBEFBAL=?,\
					DPSCNT=?,PAYCNT=?,AMOUNT=?,DPSAMT=?,TERMNO=?,\
					TAC=?,TRANSFLAG=?,CONFIRM=?,AUTHCODE=?,BATCHNO=?,\
					TERMID=?,REFNO=? WHERE TRANSDATE=? AND TRANSTIME=? \
					AND CARDNO=? AND DEVSEQNO=? AND OPERID=? \
					AND DEVPHYID=? ");

    int rc;
    sqlite_result_set_t* rs = NULL;
    rc = sql_execute(cfg_db, sql, &rs);
    if (rc)
    {
        return -1;
    }


    sqlite3_bind_text(rs->stmt, 1, record->cardphyid, -1, SQLITE_STATIC);
    sqlite3_bind_int(rs->stmt, 2, record->cardbefbal);
    sqlite3_bind_int(rs->stmt, 3, record->dpscnt);
    sqlite3_bind_int(rs->stmt, 4, record->paycnt);
    sqlite3_bind_int(rs->stmt, 5, record->amount);
    sqlite3_bind_int(rs->stmt, 6, record->dpsamt);
    sqlite3_bind_text(rs->stmt, 7, record->termno, -1, SQLITE_STATIC);
    sqlite3_bind_text(rs->stmt, 8, record->tac, -1, SQLITE_STATIC);
    sqlite3_bind_int(rs->stmt, 9, record->transflag);
    sqlite3_bind_int(rs->stmt, 10, record->confirm);
    sqlite3_bind_int(rs->stmt, 11, record->authcode);
    sqlite3_bind_int(rs->stmt, 12, record->batchno);
    sqlite3_bind_int(rs->stmt, 13, record->termid);
    sqlite3_bind_text(rs->stmt, 14, record->refno, -1, SQLITE_STATIC);
    // where
    sqlite3_bind_text(rs->stmt, 15, record->transdate, -1, SQLITE_STATIC);
    sqlite3_bind_text(rs->stmt, 16, record->transtime, -1, SQLITE_STATIC);
    sqlite3_bind_int(rs->stmt, 17, record->cardno);
    sqlite3_bind_int(rs->stmt, 18, record->devseqno);
    sqlite3_bind_int(rs->stmt, 19, record->operid);
    sqlite3_bind_text(rs->stmt, 20, record->devphyid, -1, SQLITE_STATIC);
    rc = sqlite3_step(rs->stmt);
    if (rc != SQLITE_DONE)
    {
        LOG((LOG_ERROR, "更新数据错误。rc=%d,msg=[%s]", rc, sqlite3_errmsg(cfg_db)));
        return -1;
    }
    return 0;
}

int8 trans_clear_expire_dtl(const char* daybefore)
{
    char sql[1024];
    sprintf(sql, "DELETE FROM TRANSDTL WHERE TRANSDATE<'%s' AND CONFIRM=1;", daybefore);
    int ret;
    ret = sql_exec_stmt(cfg_db, sql);
    if (ret)
        return -1;
    return 0;
}

int8 oper_get_trans(const char* transdate, int operid, oper_trans_t* opertrans)
{
    char sql[1024];
    sprintf(sql, "SELECT TRANSFLAG,COUNT(*),SUM(AMOUNT) FROM TRANSDTL WHERE TRANSDATE='%s' \
						AND OPERID=%d GROUP BY TRANSFLAG", transdate, operid);
    sqlite_result_set_t* rs = NULL;

    if (sql_execute(cfg_db, sql, &rs))
        return -1;
    opertrans->total_amt = 0;
    opertrans->total_cnt = 0;
    SAFE_STR_CPY(opertrans->transdate, transdate);
    while (!sql_fetch_resultset(rs))
    {
        //LOG( ( LOG_DEBUG, "query row..." ) );
        int transflag = sqlite3_column_int(rs->stmt, 0);
        int cnt = sqlite3_column_int(rs->stmt, 1);
        int amount = sqlite3_column_int(rs->stmt, 2);
        if (transflag == TRANS_WRITE_CARD_OK ||
                transflag == TRANS_SYS_OK ||
                transflag == TRANS_WRITE_CARD_FAILED ||
                transflag == TRANS_WRITE_CARD_UNCONFIRM)
        {
            opertrans->success_amt += amount;
            opertrans->success_cnt += cnt;
        }
        opertrans->total_amt += amount;
        opertrans->total_cnt += cnt;
    }

    sql_free_resultset(rs);
    return 0;

}
int8 trans_clear_dtl()
{
    char sql[1024];
    strcpy(sql, "DELETE FROM TRANSDTL;");
    int ret;
    ret = sql_exec_stmt(cfg_db, sql);
    if (ret)
        return -1;
    return 0;
}
int8 trans_get_oper_devseqno(p16_transdtl_t* record, int operid, int devseqno)
{
    char sql[1024];
    sprintf(sql, "SELECT TRANSDATE,TRANSTIME,CARDNO,DEVPHYID,\
					DEVSEQNO,OPERID,CARDPHYID,CARDBEFBAL,DPSCNT,PAYCNT,AMOUNT,DPSAMT,TERMNO,\
					TAC,TRANSFLAG,CONFIRM,AUTHCODE,BATCHNO,TERMID,REFNO FROM TRANSDTL WHERE \
					OPERID=%d AND DEVSEQNO=%d", operid, devseqno);
    sqlite_result_set_t* rs = NULL;
    int rc;
    if (sql_execute(cfg_db, sql, &rs))
        return -1;

    if (!sql_fetch_resultset(rs))
    {
        SAFE_STR_CPY(record->transdate, sqlite3_column_text(rs->stmt, 0));
        SAFE_STR_CPY(record->transtime, sqlite3_column_text(rs->stmt, 1));
        record->cardno = sqlite3_column_int(rs->stmt, 2);
        SAFE_STR_CPY(record->devphyid, sqlite3_column_text(rs->stmt, 3));
        record->devseqno = sqlite3_column_int(rs->stmt, 4);
        record->operid = sqlite3_column_int(rs->stmt, 5);
        SAFE_STR_CPY(record->cardphyid, sqlite3_column_text(rs->stmt, 6));
        record->cardbefbal = sqlite3_column_int(rs->stmt, 7);
        record->dpscnt = sqlite3_column_int(rs->stmt, 8);
        record->paycnt = sqlite3_column_int(rs->stmt, 9);
        record->amount = sqlite3_column_int(rs->stmt, 10);
        record->dpsamt = sqlite3_column_int(rs->stmt, 11);
        SAFE_STR_CPY(record->termno, sqlite3_column_text(rs->stmt, 12));
        SAFE_STR_CPY(record->tac, sqlite3_column_text(rs->stmt, 13));
        record->transflag = sqlite3_column_int(rs->stmt, 14);
        record->confirm = sqlite3_column_int(rs->stmt, 15);
        record->authcode = sqlite3_column_int(rs->stmt, 16);
        record->batchno = sqlite3_column_int(rs->stmt, 17);
        record->termid = sqlite3_column_int(rs->stmt, 18);
        SAFE_STR_CPY(record->refno, sqlite3_column_text(rs->stmt, 19));
        rc = 1;
    }
    else
    {
        LOG((LOG_ERROR, "无流水记录"));
        rc = 0;
    }
    sql_free_resultset(rs);
    return rc;
}

int8 trans_get_last_succ(p16_transdtl_t* record, int operid)
{
    char sql[1024];
    sprintf(sql, "SELECT TRANSDATE,TRANSTIME,CARDNO,DEVPHYID,\
					DEVSEQNO,OPERID,CARDPHYID,CARDBEFBAL,DPSCNT,PAYCNT,AMOUNT,DPSAMT,TERMNO,\
					TAC,TRANSFLAG,CONFIRM,AUTHCODE,BATCHNO,TERMID,REFNO FROM TRANSDTL WHERE \
					OPERID=%d AND TRANSFLAG=%d ORDER BY TRANSDATE DESC,TRANSTIME DESC,DEVSEQNO DESC",
            operid, TRANS_WRITE_CARD_OK);
    sqlite_result_set_t* rs = NULL;
    int rc;
    if (sql_execute(cfg_db, sql, &rs))
        return -1;

    if (!sql_fetch_resultset(rs))
    {
        SAFE_STR_CPY(record->transdate, sqlite3_column_text(rs->stmt, 0));
        SAFE_STR_CPY(record->transtime, sqlite3_column_text(rs->stmt, 1));
        record->cardno = sqlite3_column_int(rs->stmt, 2);
        SAFE_STR_CPY(record->devphyid, sqlite3_column_text(rs->stmt, 3));
        record->devseqno = sqlite3_column_int(rs->stmt, 4);
        record->operid = sqlite3_column_int(rs->stmt, 5);
        SAFE_STR_CPY(record->cardphyid, sqlite3_column_text(rs->stmt, 6));
        record->cardbefbal = sqlite3_column_int(rs->stmt, 7);
        record->dpscnt = sqlite3_column_int(rs->stmt, 8);
        record->paycnt = sqlite3_column_int(rs->stmt, 9);
        record->amount = sqlite3_column_int(rs->stmt, 10);
        record->dpsamt = sqlite3_column_int(rs->stmt, 11);
        SAFE_STR_CPY(record->termno, sqlite3_column_text(rs->stmt, 12));
        SAFE_STR_CPY(record->tac, sqlite3_column_text(rs->stmt, 13));
        record->transflag = sqlite3_column_int(rs->stmt, 14);
        record->confirm = sqlite3_column_int(rs->stmt, 15);
        record->authcode = sqlite3_column_int(rs->stmt, 16);
        record->batchno = sqlite3_column_int(rs->stmt, 17);
        record->termid = sqlite3_column_int(rs->stmt, 18);
        SAFE_STR_CPY(record->refno, sqlite3_column_text(rs->stmt, 19));
        rc = 1;
    }
    else
    {
        LOG((LOG_ERROR, "无流水记录"));
        rc = 0;
    }
    sql_free_resultset(rs);
    return rc;
}

///////////////////////////////////////////////////////////////////////////////
int8 syspara_get_feerate(int feetype, syspara_feerate_t* feerate)
{
    char sql[1024];
    sprintf(sql, "SELECT FEETYPE, FEERATE, FEEFLAG FROM DPSFEERATE WHERE FEETYPE=%d;",
            feetype);
    sqlite_result_set_t* rs = NULL;
    int rc;
    if (sql_execute(cfg_db, sql, &rs))
        return DB_ERROR;

    if (!sql_fetch_resultset(rs))
    {
        feerate->feetype = sqlite3_column_int(rs->stmt, 0);
        feerate->feerate = sqlite3_column_int(rs->stmt, 1);
        feerate->feeflag = sqlite3_column_int(rs->stmt, 2);
        rc = DB_OK;
    }
    else
    {
        rc = DB_NOTFOUND;
    }
    sql_free_resultset(rs);
    return (int8)rc;
}
int8 syspara_clear_feerate()
{
    char sql[1024];
    sprintf(sql, "DELETE FROM DPSFEERATE;");
    int ret;
    ret = sql_exec_stmt(cfg_db, sql);
    if (ret)
        return DB_ERROR;
    return DB_OK;
}
int8 syspara_add_feerate(const syspara_feerate_t* feerate)
{
    char sql[1024];
    strcpy(sql, "INSERT INTO DPSFEERATE(FEETYPE, FEERATE, FEEFLAG)VALUES(?,?,?);");

    int rc;
    sqlite_result_set_t* rs = NULL;
    rc = sql_execute(cfg_db, sql, &rs);
    if (rc)
    {
        return -1;
    }
    sqlite3_bind_int(rs->stmt, 1, feerate->feetype);
    sqlite3_bind_int(rs->stmt, 2, feerate->feerate);
    sqlite3_bind_int(rs->stmt, 3, feerate->feeflag);

    rc = sqlite3_step(rs->stmt);
    sql_free_resultset(rs);
    if (rc != SQLITE_DONE)
    {
        LOG((LOG_ERROR, "增加数据错误。rc=%d,msg=[%s]", rc , sqlite3_errmsg(cfg_db)));
        return DB_ERROR;
    }
    return DB_OK;
}
int syspara_add_allowance_detail(p16_transdtl_t* dtl, allowance_info_t* info)
{
    char sql[1024];
    strcpy(sql, "INSERT INTO ALLOWANCEDETAIL(TRANSDATE, DEVPHYID, DEVSEQNO, OPERID, \
        SUBSIDYNO, AMOUNT) VALUES(?,?,?,?,?,?);");
    int rc;
    int row = 0;
    for (row = 0; row < info->allowance_count; ++row)
    {
        sqlite_result_set_t* rs = NULL;
        rc = sql_execute(cfg_db, sql, &rs);
        if (rc)
        {
            return -1;
        }
        sqlite3_bind_text(rs->stmt, 1, dtl->transdate, -1, SQLITE_STATIC);
        sqlite3_bind_text(rs->stmt, 2, dtl->devphyid, -1, SQLITE_STATIC);
        sqlite3_bind_int(rs->stmt, 3, dtl->devseqno);
        sqlite3_bind_int(rs->stmt, 4, dtl->operid);
        sqlite3_bind_int(rs->stmt, 5, info->detail[row].subsidyno);
        sqlite3_bind_int(rs->stmt, 6, info->detail[row].amount);
        rc = sqlite3_step(rs->stmt);
        sql_free_resultset(rs);
        if (rc != SQLITE_DONE)
        {
            LOG((LOG_ERROR, "不能保存补助明细"));
            return DB_ERROR;
        }
    }
    return 0;
}
int syspara_get_allowance_detail(p16_transdtl_t* dtl, allowance_info_t* info)
{
    char sql[1024];
    sprintf(sql, "SELECT COUNT(*) FROM ALLOWANCEDETAIL WHERE DEVPHYID='%s' \
            AND DEVSEQNO=%d AND OPERID=%d AND TRANSDATE='%s'", dtl->devphyid,
            dtl->devseqno, (int)dtl->operid, dtl->transdate);
    sqlite_result_set_t* rs = NULL;
    int rc;
    if (sql_execute(cfg_db, sql, &rs))
        return DB_ERROR;

    if (!sql_fetch_resultset(rs))
    {
        info->allowance_count = sqlite3_column_int(rs->stmt, 0);
    }
    else
    {
        info->allowance_count = 0;
    }
    sql_free_resultset(rs);
    if (info->allowance_count <= 0)
        return DB_NOTFOUND;

    sprintf(sql, "SELECT SUBSIDYNO, AMOUNT FROM ALLOWANCEDETAIL WHERE DEVPHYID='%s' \
            AND DEVSEQNO=%d AND OPERID=%d AND TRANSDATE='%s'", dtl->devphyid,
            dtl->devseqno, (int)dtl->operid, dtl->transdate);
    if (sql_execute(cfg_db, sql, &rs))
        return DB_ERROR;

    info->detail = (allowance_detail_t*)malloc(sizeof(allowance_detail_t) *
                   info->allowance_count);
    int row;
    while (!sql_fetch_resultset(rs))
    {
        if (row >= info->allowance_count)
        {
            rc = DB_ERROR;
            break;
        }
        info->detail[row].subsidyno = sqlite3_column_int(rs->stmt, 0);
        info->detail[row].amount = sqlite3_column_int(rs->stmt, 1);
        rc = DB_OK;
        ++row;
    }
    sql_free_resultset(rs);
    return (int8)rc;
}
///////////////////////////////////////////////////////////////////////////////

