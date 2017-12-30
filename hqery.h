#pragma once
#include "stdafx.h"

#include "pack.hpp"

#include <string>
#include <vector>

#include <iostream>

#define MSRQ_OFS_TOSTM (sizeof(int) + sizeof(short))
#define HEADER_SIZE sizeof(pchd)

using namespace boost::asio;
using namespace boost::asio::ip;
using namespace std;

typedef boost::asio::ip::tcp::socket nsocket; 
const std::string table_name = "dbt.db";

struct pc_hd;
typedef struct pc_hd pchd;

#define Arg0 0x00000001
#define Arg1 0x00000010
#define Arg2 0x00000100
#define Arg3 0x00001000

#define SPC " "
#define VALS "VALUES"
#define COMA ","

#define ROW_ID -2

typedef boost::array<int, 10> intArrayTen;
typedef std::string stringStd;
typedef std::list<int> intList;
enum checkListIndex
{
	messageArr = 0,

	tableIvt = 1,
	connectionReq = 2,

	mailArr = 3

};
struct S
{
	static std::map<int, std::string> create_map()
	{
		std::map<int, std::string> m;
		m[checkListIndex::messageArr] = "Message Arrived";
		m[checkListIndex::mailArr] = "Mail Arrived";
		m[checkListIndex::connectionReq] = "Connection Requested";
		m[checkListIndex::tableIvt] = "Invite Requested";
		return m;
	}
	static const std::map<int, int> stringMap;

};

enum Compare
{
	Higher = 0,
	EqualAndHigher = 1,
	Equal = 2,
	EqualAndLower = 3,
	Lower = 4
};

enum SelectTarget
{
	RowId = -2,
	All = -1


};

struct Q
{
	static const char* name;
	static sqlite3* DB;
	static sqlite3* init()
	{
		sqlite3* D;
		sqlite3_open( Q::name , &D);
		
		return D;
	};

};
class query_dsc
{
public :
	query_dsc(std::string _table_name, std::vector<std::string > cont) :
		table_name_(_table_name)
	{
		auto i = cont.begin();
		for (; i != cont.end(); i++)
			column_tag_cont.push_back(*i);

		column_num = cont.size();
	};

public :
	std::string get_column_name(int indx) const { return column_tag_cont[indx]; };
	std::string get_table_name() const { return table_name_; };
	
	void add_column(std::string s) { column_num++; column_tag_cont.push_back(s); };
private:

	std::string table_name_;
	int column_num;
	std::vector<std::string> column_tag_cont;

};

std::string to_string_s(std::string s);
std::string to_string_s(int s);
typedef std::list<int> intList;
typedef std::list<std::string> stringList;
typedef std::vector<std::string> stringCont;

enum queryDescriptor
{
	User = 0,
	Table = 1,
	Message = 2,
	Participate = 3,
	UserPair = 4,
	CheckList = 5,
	Invite = 6,
	ConnectionRequest = 7
};

typedef queryDescriptor qDescriptor;

const query_dsc UserDsc("user", stringCont({ "id","pw" }));
const query_dsc MsDsc("ms", stringCont({ "tableKey", "sender","message", "time", "chk" }));
const query_dsc TableDsc("chatTable", std::vector<std::string>({  "host", "participations" }));
const query_dsc PartDsc("part", std::vector<std::string>({"tableKey","user"}));
const query_dsc PairDsc("userPair", std::vector<std::string>({"user0", "user1"}));
const query_dsc CheckListDsc("checkList", std::vector<std::string>({ "user","messageArr","tableIvt", "connctionReq" }));
const query_dsc InviteDsc("invite", std::vector<std::string>({"target", "tableKey", "sender" }));
const query_dsc ConnectionRequestDsc("connectionRequest", std::vector<std::string>({"sender","receiver","time"}));

const std::vector<const query_dsc*> descriptorCont 
	= { &UserDsc, &TableDsc , &MsDsc ,& PartDsc, &PairDsc ,&CheckListDsc , &InviteDsc, &ConnectionRequestDsc};
const std::vector<std::string> eqivDscCont = { ">" , ">=" , "==" , "<" , "<=" };

typedef struct
{
	int tableKey;

	short userIndex;
	short text_lenth;

	int arrived;
	short check;

	int sz;
	boost::posix_time::ptime time;

}msOut;

typedef std::pair< pBuffer* , int*> queryContainer;
typedef std::list< std::pair< msOut, pBuffer >> mPairList;
template <class A>

std::string chanege_condition_insert_query(std::string& query, queryDescriptor qd , pair<int, A> a )
{
	std::string finder = "VALUES(";

	int start_delete = query.find_first_of(finder, finder.size());

	for (int i = 0; i < a.first; i++)
	{

	}
	
	return query;
}
template <class T0 , class T1 , class T2 , class T3>
std::string insert_query_with_pair(qDescriptor qd , pair <int,T0> a0, pair<int,T1> a1, pair<int,T2> a2 , std::pair<int , T3> a3)
{

	std::string query = "INSERT INTO ";
	query.reserve(256);

	const query_dsc* cont = descriptorCont.at(qd);
	query += cont->get_table_name();
	query += " (";
	query += cont->get_column_name(a0.first);
	query += " , ";
	query += cont->get_column_name(a1.first);
	query += ",";
	query += cont->get_column_name(a2.first);
	query += ",";
	query += cont->get_column_name(a3.first);
	query += ") VALUES (";

	query += to_string_s(a0.second);
	query += " , ";
	query += to_string_s(a1.second);
	query += ",";
	query += to_string_s(a2.second);
	query += ",";
	query += to_string_s(a3.second);
	query += ");";

	return query;

}

template <class T0 , class T1 , class T2>
std::string insert_query_with_pair( qDescriptor qd , pair < int , T0> a0 ,pair<int , T1> a1,  pair<int , T2 > a2)
{
	std::string query = "INSERT INTO ";
	query.reserve(512);

	const query_dsc* cont = descriptorCont.at(qd);
	query += cont->get_table_name();
	query += " (";
	query += cont->get_column_name(a0.first);
	query += " , ";
	query += cont->get_column_name(a1.first);
	query += ",";
	query += cont->get_column_name(a2.first);

	query += ") VALUES (";

	query += to_string_s(a0.second);
	query += " , ";
	query += to_string_s(a1.second);
	query += ",";
	query += to_string_s(a2.second);

	query += ");";

	return query;
}

template <class T0, class T1>
std::string insert_query_with_pair( qDescriptor qd , pair<int ,T0 > a0 , pair<int, T1> a1 )
{
	std::string query = "INSERT INTO ";
	query.reserve(512);

	const query_dsc* cont = descriptorCont.at(qd);
	query += cont->get_table_name();
	query += " (";
	query += cont->get_column_name(a0.first);
	query += " , ";
	query += cont->get_column_name(a1.first);
	query += ") VALUES (";
	query += to_string_s(a0.second);
	query += " , ";
	query += to_string_s(a1.second);
	query += ");";

	return query;

}

template <class A0 >
std::string select_query(queryDescriptor qd, pair<int, A0 > a0, short e0,  int target)
{
	std::string Tagt;
	std::string Cond;

	const query_dsc* qdsc = descriptorCont[qd];

	if (target == -1)
		Tagt = "*";
	else if (target == -2)
		Tagt = "rowid";

	else
		Tagt = qdsc->get_column_name(target);

	if (a0.first != -2)
		Cond = qdsc->get_column_name(a0.first);
	else
		Cond = "rowid";

	Cond += eqivDscCont[e0];
	Cond += to_string_s(a0.second);

	std::string s = "SELECT ";
	s += Tagt;
	s += " FROM ";
	s += qdsc->get_table_name();
	s += " WHERE( ";
	s += Cond;
	s += " ) ;";

	return s;

}




template <class A0 >
void add_condition_to_insert_query(qDescriptor qd , stringStd* query , A0 a0)
{
	const query_dsc* qdsc = descriptorCont[qd];

	int pos = query->find_last_of(")");

	stringStd stringAdded = " , ";

	stringAdded += to_string_s(a0);

	query->insert(pos, stringAdded);
}




template <class A0 , class A1>
void add_condition_to_insert_query(qDescriptor qd , stringStd* query , A0 a0 , A1 a1)
{
	
	const query_dsc* qdsc = descriptorCont[qd];

	int pos = query->find_last_of(")");
	
	stringStd stringAdded = " , ";

	stringAdded += to_string_s(a0);

	stringAdded += " , ";

	stringAdded += to_string_s(a1);
	
	query->insert(pos , stringAdded);

	
}

template <class A0, class A1>
std::string select_query(queryDescriptor qd , pair< int , A0 > a0 , pair<int ,A1> a1 , short e0 , short e1, int target)
{
	std::string Tagt;
	std::string Cond;

	std::string Tagt0;
	std::string Tagt1;
	
	const query_dsc* qdsc = descriptorCont[qd];

	if (a0.first == ROW_ID)
		Tagt0 = "rowid";
	else
		Tagt0 = qdsc->get_column_name(a0.first);

	if (a1.first == ROW_ID)
		Tagt1 = "rowid";
	else
		Tagt1 = qdsc->get_column_name(a1.first);

	if (target == -1)
		Tagt = "*";
	else if (target == -2)
		Tagt = "rowid";

	else
		Tagt = qdsc->get_column_name(target);

		Cond = Tagt0;
		Cond += eqivDscCont[e0];
		Cond += to_string_s(a0.second);
		Cond += " AND ";
		Cond += Tagt1;
		Cond += eqivDscCont[e1];
		Cond += to_string_s(a1.second);

		std::string s = "SELECT ";
		s += Tagt;
		s += " FROM ";
		s += qdsc->get_table_name();
		s += " WHERE( ";
		s += Cond;
		s += " ) ;";

	return s;
}

template < class A0, class A1 >
std::string insert_query(queryDescriptor qd, A0 a0, A1 a1)
{

	std::string s;
	s.resize(100);
	auto cont = descriptorCont[qd];

	s = "INSERT INTO ";
	s += cont->get_table_name();

	s += SPC;
	s += VALS;
	s += "( ";

	s += to_string_s(a0);

	s += COMA;
	s += to_string_s(a1);
	s += ");";

	return s;	
}

typedef struct
{
	int tableKey;
	std::string text;

	int arrived;

}tableQry;



typedef struct
{
	short last;
	int size_of_back;

} tail;

typedef struct qry
{
	int kind;
	int key;

} qryst;

typedef struct
{
	int sizeof_id;
	int sizeof_pw;


} rqlog;


typedef struct participation
{
	int key;

	int tbk;
	int usk;
}part;

typedef struct mail
{
	int sd;
	int rc;

	std::string s;

	int time;

} mi;

typedef struct msg
{
	int table_key;
	int sender;

	short sz;

	int uncheck;
	boost::posix_time::ptime time;
} msReq;

typedef struct
{
	int table_key;
	int target_id_size;

	
}ivtReq;

typedef struct table_st
{
	int key;
	int user_num;
	int arrived;

} dtable;

typedef struct
{
	int table_key;
	int sender_id_size;
	char time[20];
	


}strtIvtResponse;


enum eAction
{
	ReqLog = 20,
	ReqLogOut = 21,

	ReqInit = 50,
	
	SendMsg = 80,
	
	RequestConnection = 81,
	RequestInvite = 82,
	RequestCreateTable = 83,

	ConnectionClese = 99
};

enum eCommand
{
	GetMsg = 70,
	GetIvt = 71,
	GetConnection = 72,
	GetFriendList = 73
};


typedef struct
{
	int host;
	short user_num;

	tail tl;

} tbrq;

typedef struct
{
	int tablekey;
	int host;
	int user_num;
	char* ustream;

} tbccb;


typedef struct
{
	int usk;
	int ms_ch_rid;
	int dt_ch_rid;

} usif;


typedef struct ivt
{
	int tkey;
	int uk;
	int num;

	time_t time;

}ivts;

typedef struct
{
	int key;
	std::string pw;

}user;

typedef struct msgpassor
{
	int table_key;
	int sd;
	int time;
	int requ_num;

	int size;
	short init;
}msps;

typedef struct
{
	int sender_id_size;
	int receiver_id_size;
	boost::posix_time::ptime time;
}concResponse;

typedef struct
{
	int sender_id_size;
	int receiver_id_size;
	boost::posix_time::ptime time;

}concRequest;

typedef struct
{
	int host_id_size;
	int participations;
	
}createTableReq;
typedef struct
{
	int key_allocated;
	

}createTableResponse;
typedef struct 
{
	int id_size;
	int id_key;
}UserDescriptor;


namespace Qr
{
	mPairList* get_new_message(sqlite3* db, int last_check, std::string host);
	
	void make_connection(sqlite3* db, int target_id , int rk);
	void make_participation(sqlite3* db, int table_key);
	
	bool openDB(sqlite3** db, std::string s);
	bool closeDB(sqlite3* db );

	int insert_message(sqlite3* db, msReq& rq, std::string s);
	int insert_table_key(sqlite3* db,tbrq rq , int* arr);
	int insert_connection(sqlite3* db, std::string sender, std::string target, short rk);
	int insert_participation(sqlite3* db, int uid, int table_key);
	
	int create_table(sqlite3* db, std::string host , std::list<std::string>& cont );
	
	void insert_invite(sqlite3* db , std::string target, std::string sender , int tableKey);
	int insert_participation(sqlite3* db, std::string uid, int table_key, int index);

	user find_user(sqlite3* db,std::string id);
	bool check_user_password(sqlite3* db, const std::string& id, const std::string& pw);

	std::list<int>* get_table(sqlite3* db, std::string user);
	int get_new_message(sqlite3* db, pBuffer& ,std::string user, int lastCheck);
	int get_new_message_in_table(sqlite3* db, pBuffer& ,int tableKey, int& lastCheck);
	intArrayTen* get_check_list(sqlite3* db, std::string user);

	int get_new_ivt_req(sqlite3* db, std::string user, pBuffer& pbuf, int& lastCheck);
	std::list<concResponse>* get_new_connection_req(sqlite3* db, std::string user, int& lastCheck);
	int get_new_connection_req(sqlite3* db, std::string user, pBuffer& p , int& lastCheck);

	std::list<std::string>* get_friend_list(sqlite3* db, std::string user);
	std::list<std::string>* get_dining_members(sqlite3* db, int tableKey);
};

namespace task
{
	int logc_msrq(char* src, std::string& s);
	void logc_login(char* src, std::string& id, std::string& pw);

	void set_result(boost::optional<boost::system::error_code>* a, boost::system::error_code b);

};
