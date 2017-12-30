#include "stdafx.h"

#include "hqery.h"
#include "pack.hpp"

using namespace boost;

const char* Q::name = "dbt.db"; sqlite3* Q::DB = Q::init();

std::wstring to_wsting_s(std::wstring s) { return std::wstring(); }


std::string to_string_s(std::string s) 
{
	 std::string res = "\""; res += s; res
	+= "\"";

	return  res;
 }

template <>
std::string insert_query_with_pair( qDescriptor qd , pair<int , std::string> a0 , pair<int,std::string> a1)
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
std::string to_string_s(int s) { return std::to_string(s); }

bool Qr::check_user_password(sqlite3* db , const std::string& id , const
std::string& pw) { std::string query = select_query(queryDescriptor::User,
pair< int, std::string >( 0 , id ) , 2 , 1);

	sqlite3_stmt *statement; std::string wpw; char* err;

	sqlite3_exec(db, query.c_str(), [](void* pw, int argc, char** result,
char**)->int { std::string* rpw = (std::string*)pw;
		
		*rpw = result[0];
		
		return 0; }, &wpw , &err);
	

	if (pw == wpw) { return true; }

	return false;

}

user Qr::find_user(sqlite3* db, std::string id )  { user us = {}; return us; }


void Qr::insert_invite(sqlite3* db , std::string target , std::string sender ,
int tableKey) { char* err; std::string query =
insert_query_with_pair(qDescriptor::Invite , std::pair<int, std::string> (0,
target), std::pair<int, int > (1, tableKey) , std::pair<int ,std::string >
(2,sender));

	sqlite3_exec(db , query.c_str() , 0 , 0, &err);

	if (err)
	 { BOOST_LOG_TRIVIAL(info) << err; }

}

int Qr::insert_table_key(sqlite3* db, tbrq rq, int* arr) { return 0; }

int Qr::create_table(sqlite3* db, std::string host, std::list<std::string>& cont)
{
	int participations = cont.size(); std::string query = insert_query(qDescriptor::Table , host , participations); 
	char* err;
	sqlite3_exec(db, query.c_str(), 0, 0, &err);
	
	if(err)
		{ BOOST_LOG_TRIVIAL(info) << err; } 

	int row_id = sqlite3_last_insert_rowid(db); int index = 0;
	
	BOOST_LOG_TRIVIAL(trace) << "Table Create : " << row_id; 

	for(auto it = cont.begin() ; it != cont.end() ; it++, index ++)
	{ 
		query = insert_query(qDescriptor::Participate , *it ,row_id );
		add_condition_to_insert_query( qDescriptor::Participate , &query, index  );
		sqlite3_exec( db , query.c_str() , 0 , 0, &err  );
	 } 

	return row_id;
}

int Qr::insert_connection(sqlite3* db, std::string sender, std::string target,short rk)
{

	std::string query = insert_query_with_pair(qDescriptor::ConnectionRequest, pair<int ,std::string>(0,sender) , pair<int , std::string >(1,target));
		
	char* err;

	sqlite3_exec(db, query.c_str() , 0, 0, &err);

	if (err) 
	{ BOOST_LOG_TRIVIAL(info) << err; }

	return 0; }

int Qr::insert_participation(sqlite3* db, std::string uid, int table_key, int
index)
 { 
	std::string query = insert_query(qDescriptor::Participate,table_key, uid ); add_condition_to_insert_query(qDescriptor::Participate,&query, index);

	char* err;

	sqlite3_exec(db, query.c_str(), 0, 0, &err);

	if (err)
	 { BOOST_LOG_TRIVIAL(info) << err; }
	
	return sqlite3_last_insert_rowid(db);

}



bool Qr::openDB(sqlite3** db, std::string s) { int rc =
sqlite3_open(table_name.c_str(), db);

	if (rc != SQLITE_OK) { sqlite3_close(*db); return false; }

	return true; }

bool Qr::closeDB(sqlite3* db) { return (sqlite3_close(db) == SQLITE_OK); }

void task::logc_login(char* src, std::string& id, std::string& pw) { rqlog r =
{}; memcpy(&r , src , sizeof(rqlog));

	id.resize(r.sizeof_id); pw.resize(r.sizeof_pw);

	memcpy( (void*)id.c_str() , src + sizeof(rqlog) , r.sizeof_id);
memcpy((void*)pw.c_str(), src + sizeof(rqlog) + r.sizeof_id, r.sizeof_pw);


}

int task::logc_msrq(char* src, std::string& s) {

	return 0; }

int Qr::get_new_message_in_table(sqlite3* db , pBuffer& p , int tableKey , int&
lastCheck) { char* err;

	std::string query = select_query(qDescriptor::Message, pair<int, int >(
0, tableKey ),pair<int,int>(-2,lastCheck) , 2, 0, -1);

	int count = 0;
	pair<pBuffer*, int* > result( &p , &count );

	sqlite3_exec(db, query.c_str(), [](void* cont, int argc, char** argv,
char** err) -> int {
	
		pBuffer* b_cont = (pBuffer*) ( (pair<pBuffer*, int* >*) cont
)->first; int* ref = ((pair<pBuffer*, int* >*) cont)->second;

		std::string text = argv[0]; std::string sender = argv[1];
		
		msOut r = {}; r.tableKey = stoi(argv[0]); r.text_lenth =
text.size();
		r.time = boost::posix_time::ptime( boost::posix_time::time_from_string(std::string(argv[3] ,20)));

		r.sz = r.text_lenth;

		b_cont->allocate_some(  &r, sizeof(msOut)  );
b_cont->allocate_some((void*)text.c_str(), text.size());
	
		(*ref)++;

		return 0;

	}, &result, &err);


	lastCheck = sqlite3_last_insert_rowid(db);

	return count; }

int Qr::get_new_message(sqlite3* db ,pBuffer& p,  std::string user, int
lastCheck ) {	auto list = get_table(db, user); auto it = list->begin();

	int sum = 0;

	for (; it != list->end(); it++) sum += get_new_message_in_table(db , p,
*it , lastCheck );
	
	return sum; }

int Qr::get_new_ivt_req(sqlite3* db, std::string user, pBuffer& pbuf ,int&
lastCheck) { std::list<strtIvtResponse> *res = new
std::list<strtIvtResponse>(); std::string query =
select_query(queryDescriptor::Invite, pair<int, std::string >(0, user),
Compare::Equal, SelectTarget::All);
	
	char* err;

	int count = 0; queryContainer cont(  &pbuf , &count );

	sqlite3_exec(db, query.c_str(), [](void* cont, int argc, char** argv,
char** cl)->int {

		auto qCont = (queryContainer*)cont; int* count = qCont->second;
pBuffer* pbuf = qCont->first; std::string sender = argv[2];

		strtIvtResponse r = {}; r.sender_id_size = sender.size();
r.table_key = stoi(argv[1]);
		
		memcpy(r.time, argv[3], 20);

		pbuf->allocate_some(&r, sizeof(strtIvtResponse));
pbuf->allocate_some((void*)sender.c_str(), r.sender_id_size);

		(*count)++; return 0; }, &cont, &err);

	lastCheck = sqlite3_last_insert_rowid(db);

	return count;

}

int Qr::get_new_connection_req(sqlite3* db , std::string user , pBuffer& pbuf ,
int& lastCheck) { int count = 0; queryContainer qCont(&pbuf	, &count);

	std::string query = select_query (queryDescriptor::ConnectionRequest,
pair<int, std::string >(1, user), pair<int, int>( -2 , lastCheck),
Compare::Equal, Compare::EqualAndHigher, SelectTarget::All); char* err;

	sqlite3_exec(db, query.c_str(), [](void* cont, int argc, char** argv,
char** cl)->int { queryContainer* qCont = (queryContainer*)cont; pBuffer* pBuf
= qCont->first; int* count = qCont->second;

		std::string sender = argv[0]; concResponse r = {};
		
		r.sender_id_size = sender.size(); memcpy(&r.time, argv[2], 20);

		pBuf->allocate_some(&r, sizeof(concResponse));
pBuf->allocate_some((void*)sender.c_str() , sender.size() );
		
		(*count)++;
		
		return 0;

	}, &qCont, &err);

	lastCheck = sqlite3_last_insert_rowid(db);

	return count;

}

std::list<int>* Qr::get_table(sqlite3* db, std::string user) { std::list<int>
*res = new std::list<int>(); char* err;

	std::string query = select_query(queryDescriptor::Participate, pair<int
, string >( 1 , user ), 2, 0);

	sqlite3_exec(db, query.c_str(), [](void* cont, int argc, char** argv,
char** err) -> int { std::list<int>* keyList = (std::list<int>*)cont;
				
		keyList->push_back(stoi(argv[0]));
		
		return 0;

	}, res , &err);

	if (err) BOOST_LOG_TRIVIAL(trace) << "DB err : " << err;

	return res; }

intArrayTen* Qr::get_check_list(sqlite3* db, std::string user ) { intArrayTen*
res = new intArrayTen();

	std::string query = select_query(qDescriptor::CheckList, pair<int,
stringStd >(0, user), Compare::Equal, SelectTarget::All); char* err;

	sqlite3_exec(db, query.c_str(), [](void* cont , int colCount ,char**
result, char** err )->int { auto list = (intArrayTen*)cont;

		for (int i = 1; i < colCount; i++) list->at(i) =
stoi(result[i]);

		return 0; } , res ,&err);

	return res; }

int Qr::insert_message(sqlite3* db , msReq& rq , std::string text) { char* err
= 0;
	
	std::string query = insert_query_with_pair(qDescriptor::Message,
std::pair<int,int>(0, rq.table_key) , std::pair<int , int> (1, rq.sender) ,
std::pair<int,std::string>(2,text), std::pair<int,int>(4,rq.uncheck));

	BOOST_LOG_TRIVIAL(trace) <<  "Message Inserted : "<< text;
sqlite3_exec(Q::DB, query.c_str() , 0, 0, &err);
	
	if (err)
	 { BOOST_LOG_TRIVIAL(info) << err; }

	return sqlite3_last_insert_rowid(db); }

std::list<std::string>*
Qr::get_dining_members(sqlite3* db, int tableKey)
{
	 std::string query = select_query(qDescriptor::Participate, pair<int, int >(0,tableKey),Compare::Equal, 1);
	
	auto list = new std::list<std::string>(); char* err = 0;

	sqlite3_exec(db, query.c_str() , [](void* cont, int colNumber, char**
result, char**)->int { auto list = (std::list<std::string>*) cont; std::string
s = result[0]; list->push_back(s); return 0; },(void*)list, &err);

	if (err)
	 { BOOST_LOG_TRIVIAL(info) << err; }

	return list;

}

std::list<std::string>* Qr::get_friend_list(sqlite3* db, std::string user)
{
	 std::string query = select_query(qDescriptor::UserPair, std::pair<int , std::string >(0 , user),Compare::Equal,  1);
	std::string query1 = select_query(qDescriptor::UserPair, std::pair<int, std::string >(1,user),Compare::Equal ,  0);

	std::list<std::string>* friendList = new std::list<std::string>() ;
	char* err = 0;

	sqlite3_exec(db, query.c_str(), [](void* cont, int argc, char** argv,char** err) -> int 
	{
		std::list<std::string>* fList =( std::list<std::string>*) cont;
		fList->push_back( argv[0]);
		return 0;
	}, friendList , &err);

	sqlite3_exec(db , query1.c_str() , [](void* cont , int argc, char** argv, char** err)->int
	{
		std::list<std::string>* fList = (std::list<std::string>*)cont;
		fList->push_back( argv[0]);
		return 0;
	}, friendList, &err);

	return friendList;
}
