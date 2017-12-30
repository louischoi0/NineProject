#pragma once

#include "nof.h"
#include "pack.hpp"

#include "hqery.h"
#include "ntask.h"

#define MAX_LEN 1024

using namespace boost;
using namespace boost::asio;

typedef boost::function<void(boost::system::error_code&)> sessionTask;
typedef const boost::system::error_code constBoostErr;

typedef struct
{
	std::string user;


}strtGetMsg;

typedef struct
{
	int user;


}user_ref;

typedef struct
{
	int uid;
	std::string pw;

}logrq;

typedef struct
{
	bool result;

}strtLogResp;

typedef struct
{
	short type;
	int check_last;

}ckst;

#define STATE_INDEX_LOGGED 0
#define STATE_INDEX_ 1

class checkOrder
{

public :
	checkOrder() {};

	int queryType;

	std::string Tag;
	int last_checked;

	int get_query_type() { return queryType; };

	bool _new;

};


class session_no_ssl :  public boost::enable_shared_from_this<session_no_ssl>
{

public :
	session_no_ssl(io_service& io, nMap* map) :
		socket_(io),
		task_end_strand(io),
		io_(io),
		pc(&data_),
		N(map),
		_idle(false),
		hBuf_(sizeof(pchd)),
		source_(1024)
	{

	}

	void run(const boost::system::error_code& error)
	{
		if (!error)
		{
			connect_callback(error);
		}
		else
		{
			BOOST_LOG_TRIVIAL(trace) << "session error" ;
		}
	}
	
	nsocket& socket() { return socket_; };

private:
	nsocket socket_;
	
	boost::array<ckst, 10> cList;
	boost::shared_ptr<std::string> req_login_id_ptr;
	boost::shared_ptr<std::string> req_login_pw_ptr;

	void check_receive_task
	(constBoostErr& err, sessionTask callabck);

	void connect_callback(const boost::system::error_code& error);
	
	void write_handle(const boost::system::error_code& err);
	void read_header_handle(const boost::system::error_code& err);

	void read_packet(const boost::system::error_code& err, void* cont);
	void read_handle(const boost::system::error_code& err);
	void request_login();

	void session_task(const boost::system::error_code& err);
	void login_callback(const boost::system::error_code& error, std::string );

	void disconnect(const boost::system::error_code& err);
	bool session_init(const boost::system::error_code& err, std::string );

	void send_Okay(const boost::system::error_code& err);
	void check_Okay(const boost::system::error_code& err);

	void renew_nof_and_check_list(const boost::system::error_code& err , checkListIndex idx, int lastCheck);

	//Response For Command
	void rget_new_message(const boost::system::error_code&);
	void rget_friend_list(const boost::system::error_code&); 
	void response_req_get_new_ivt(const boost::system::error_code& err);
	void response_req_get_new_conc(const boost::system::error_code& err);

	void response_req_get_table_mine(const boost::system::error_code& err);
	//Response For Action
	void response_act_create_chat_table(const boost::system::error_code& err, pBuffer& p);
	void response_act_send_msg(const boost::system::error_code& err, pBuffer& p);
	void response_act_connect(const boost::system::error_code& err );
	
	void response_req_login(const boost::system::error_code& err, pBuffer& h, int ,int);
	
	void task_end(const boost::system::error_code& err, int );
	void response_act_invite(const boost::system::error_code& err, pBuffer& h);
		
	void error_handle(const boost::system::error_code& err);

	void Command(const boost::system::error_code& err);
	void Action(const boost::system::error_code& err);

	void recommand(const boost::system::error_code& err, int e);
	void reaction(const boost::system::error_code& err, int e);
	void read_id_and_password(const boost::system::error_code& err, pBuffer& h);

	void initialize_nof_array();

public:

	void set_self_iterator(std::list<session_no_ssl*>::iterator self_pointer) { self = self_pointer; };
	void check_mail_box();
	void renew_check_list_and_end_task(constBoostErr& err, checkListIndex index, int row_id);
	inline void set_notifier(boost::array<nof, 10> * nf) { this->nf = nf; };
private:

	bool initialized;
	
	std::list<std::pair<msOut, std::string >>* msOutList;
	std::list<concResponse>* connectionOutList;
	
	boost::asio::strand task_end_strand;
		
	intArrayTen* checkManager;
	boost::array<char, 1024> data_;
	
	std::string user_name;
	boost::array<nof, 10>* nf;
	bool _idle;
	Packer pc;
	boost::asio::io_service& io_;
	nMap* N;
	boost::container::deque<pchd> notice_;
	std::list<session_no_ssl*>::iterator self;	

	bool renewed;
	pBuffer hBuf_;
	pBuffer source_;

};

void send_id_with_desc( nsocket& _socket , std::string s , boost::system::error_code& err);
