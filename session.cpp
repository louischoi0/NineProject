#include "stdafx.h"
#include "session.h"
using namespace boost::asio;

void nmap_notify(nMap& map, std::string user_id, int task_number)
{
	boost::optional<boost::array<nof, 10>* > cle_nof;
	
	if (cle_nof = map.get_notifier(user_id))
		(*cle_nof)->at(task_number).renew();

}

void nmap_notify_with_list(nMap& map, std::list<std::string>& a, int task_number)
{
	for (auto i = a.begin(); i != a.end(); i++)
		nmap_notify(map, *i, task_number);
}


int command_number_to_check_list_index(int command)
{
	if (command == eCommand::GetConnection)
		return 2;

	else if (command == eCommand::GetIvt)
		return 1;

	else if (command == eCommand::GetMsg)
		return 0;
	else if (command == eCommand::GetFriendList)
		return 9;

	return -1;
}

enum AC
{
	Action = 0,
	Command = 1
};


void session_no_ssl::login_callback(const boost::system::error_code& error, std::string id)
{
	//usState::usState();

}

void session_no_ssl::connect_callback(const boost::system::error_code& error)
{
	BOOST_LOG_TRIVIAL(trace) << "Client Connected";
	session_task(error);
}

void session_no_ssl::check_mail_box( )
{
	pchd h = {};
	checkListIndex task_id;
	int lastCheck = 0;

	if (!initialized)
		return;

		if ((*nf)[checkListIndex::messageArr].get_new())
		{
			h.actionOrCommand = AC::Command;
			h.sz = sizeof(msReq);
			h.comd = eCommand::GetMsg;
			h.count = 1;
			notice_.push_back(h);

			BOOST_LOG_TRIVIAL(trace) << user_name +" : Message Arrived Renewed";
			renewed = true;
			
			(*nf)[checkListIndex::messageArr].task_end();
		}


		if ((*nf)[checkListIndex::tableIvt].get_new())
		{
			h.actionOrCommand = AC::Command;
			h.sz = sizeof(msReq);
			h.comd = eCommand::GetIvt;
			h.count = 1;

			notice_.push_back(h);

			BOOST_LOG_TRIVIAL(trace) << user_name + " : Invite Request Renewed";
			renewed = true;
			(*nf)[checkListIndex::tableIvt].task_end();
		}

		if((*nf)[checkListIndex::connectionReq].get_new())
		{

			h.actionOrCommand = AC::Command;
			h.sz = sizeof(connectionReq);
			h.comd = eCommand::GetConnection;
			h.count = 1;			
			notice_.push_back(h);
			BOOST_LOG_TRIVIAL(trace) << user_name + " : Connection Request Renewd";
			
			renewed = true;
			(*nf)[checkListIndex::connectionReq].task_end();
		}
		
		if(renewed)
		{
			boost::system::error_code err;
		}	

}

void session_no_ssl::renew_nof_and_check_list(const boost::system::error_code& err, checkListIndex idx, int last_check)
{
	(*nf)[idx].task_end();
	(*checkManager)[idx] = last_check;
}

void session_no_ssl::session_task(const boost::system::error_code& err)
{
	if(err)
	
	{	
		BOOST_LOG_TRIVIAL(trace);
		error_handle(err);
	}
	else{

		if ( pc.empty() )
		{
			_idle = true;
			boost::asio::async_read(socket_, boost::asio::buffer(pc.data(), sizeof(pchd)),
				boost::bind(&session_no_ssl::read_header_handle, this, asio::placeholders::error));
		}

		else
		{
			read_handle(err);
		}

	}


}

void session_no_ssl::read_header_handle(const boost::system::error_code& err)
{
	if (err)
		error_handle(err);

		if (!pc.decode_hader(err))
		{
			session_task(err);
		}	

		else
			read_handle(err);

}

void session_no_ssl::read_id_and_password(const boost::system::error_code& err , pBuffer& h)
{
	int id_size = (int)*h.src();
	int pw_size = (int)*(h.src() + 4);
	if(!err)
	{
		pBuffer idpw(id_size + pw_size);
		
		boost::asio::async_read( socket_, boost::asio::buffer(idpw.src(), id_size + pw_size),
			boost::bind(&session_no_ssl::response_req_login, this, boost::asio::placeholders::error, idpw, id_size , pw_size));
	}	
	else
	{
		error_handle(err);
	}
}



void session_no_ssl::response_req_login(const boost::system::error_code& err, pBuffer& h, int id_size, int pw_size)
{
	if(err)
	{
		error_handle(err);
		return ;
	}
	
	std::string req_id;
	std::string req_pw;
	req_id.resize(id_size);
	req_pw.resize(pw_size);
	
	memcpy( (void*)req_id.c_str() , h.src() , id_size  );
	memcpy( (void*)req_pw.c_str(), h.src() + id_size, pw_size);

	strtLogResp response = {};

	if (Qr::check_user_password(Q::DB, req_id, req_pw))
	{
		response.result = true;
		
		session_init(err, req_id);
		BOOST_LOG_TRIVIAL(trace) << "Client logged User Id : " << user_name;
	}

	else
	{
		BOOST_LOG_TRIVIAL(trace) << "Incorrect Log in Requested";
		response.result = false;
	}

	pchd hd = pc.front();
	
	hd.ack = 1;
	hd.sz = sizeof(strtLogResp);

	if(err)
	{
		error_handle(err);
		return;
	}
	
		boost::asio::async_write(socket_, boost::asio::buffer(&response, sizeof(strtLogResp)),
			boost::bind(&session_no_ssl::task_end, this, boost::asio::placeholders::error, 0));

}
	
void session_no_ssl::task_end(const boost::system::error_code& err, int i)
{
	try
	{	
		pc.pop();
		source_.init_record();
		session_task(err);
	
	}
	
	catch (const boost::system::error_code& err)
	{
		error_handle(err);
	}

}

void session_no_ssl::Command(const boost::system::error_code& err)
{	
	pchd h = pc.front();

	int index = command_number_to_check_list_index(h.comd);
	int lastCheck = (*checkManager)[index];

	if (h.comd == eCommand::GetConnection)
		response_req_get_new_conc(err);

	else if (h.comd == eCommand::GetIvt)
		response_req_get_new_ivt(err);
	
	else if (h.comd == eCommand::GetMsg)
		rget_new_message(err);

	else if ( h.comd == eCommand::GetFriendList)
		rget_friend_list(err);		

}
void session_no_ssl::rget_friend_list(const boost::system::error_code& err)
{
	std::list<std::string>* fList = Qr::get_friend_list(Q::DB,user_name);

	pchd h = {};

	h.ack = 1;
	h.count = fList->size();

	auto fiter =  fList->begin();

	boost::system::error_code ell;
	boost::asio::write( socket_ , boost::asio::buffer(&h, sizeof(pchd)));

	int i = 0;
	for(i = 0; i < fList->size() ; i++)
	{
		send_id_with_desc( socket_ , *(fiter++), ell);

		if(ell)
			error_handle(ell);
	};

	task_end(ell,0);

}
void send_id_with_desc( nsocket& _socket , std::string s , boost::system::error_code& err)
{
	UserDescriptor uDesc = {};
	uDesc.id_size = s.size();
	
	boost::asio::write(_socket, boost::asio::buffer( &uDesc , sizeof(UserDescriptor)), err);
	boost::asio::write(_socket, boost::asio::buffer( s.c_str() , s.size() ), err);

}  
void session_no_ssl::Action(const boost::system::error_code& err)
{
	pchd h = pc.front();

	if (h.comd == eAction::ReqLog)
	{
		boost::asio::async_read(socket_, boost::asio::buffer( hBuf_.src(), sizeof(rqlog)),
			boost::bind(&session_no_ssl::read_id_and_password, this, boost::asio::placeholders::error, hBuf_));
	}
	else if (h.comd == eAction::SendMsg)
		boost::asio::async_read(socket_, boost::asio::buffer(hBuf_.src(), sizeof(msReq)),
			boost::bind(&session_no_ssl::response_act_send_msg, this, boost::asio::placeholders::error, hBuf_));

	else if (h.comd == eAction::RequestConnection)
	{
		boost::asio::async_read(socket_, boost::asio::buffer( hBuf_.src() , sizeof(concRequest)),
			boost::bind(&session_no_ssl::response_act_connect, this, boost::asio::placeholders::error));
	}

	else if (h.comd == eAction::RequestInvite)
		boost::asio::async_read(socket_, boost::asio::buffer(hBuf_.src() , sizeof(ivtReq)),
			boost::bind(&session_no_ssl::response_act_invite, this, boost::asio::placeholders::error, hBuf_));

	else if (h.comd == eAction::RequestCreateTable)
		boost::asio::async_read(socket_, boost::asio::buffer(hBuf_.src(), sizeof(createTableReq)),
			boost::bind(&session_no_ssl::response_act_create_chat_table, this , boost::asio::placeholders::error, hBuf_));

}

void session_no_ssl::response_act_create_chat_table(const boost::system::error_code& err, pBuffer& p)
{
	createTableReq req = *(createTableReq*)hBuf_.src();
	createTableResponse res = {	};

	int host_id_size = req.host_id_size;
	int participations = req.participations;
	int tableCreatedRowid = -1;

	BOOST_LOG_TRIVIAL(trace) << "PARTICIPATIONS : " << participations;
		
	std::list<std::string> cont;
	char data[256];
	for( int i = 0; i < participations ; i ++)
	{	
		UserDescriptor us;
		std::string s;

		boost::asio::read(socket_ , boost::asio::buffer( &us, sizeof(UserDescriptor)  ));
		boost::asio::read(socket_, boost::asio::buffer(data , us.id_size));
		
		s = std::string(data ,us.id_size );
		cont.push_back(s);
		BOOST_LOG_TRIVIAL(trace) << "INVITED : " << s;
	}
			
	res.key_allocated = Qr::create_table(Q::DB, user_name , cont );	
	boost::asio::write(socket_, boost::asio::buffer(&res, sizeof(createTableResponse)));
	nmap_notify_with_list(*N, cont, checkListIndex::tableIvt);
	task_end(err, 0);
}

void session_no_ssl::response_act_connect(const boost::system::error_code& err)
{
	concRequest rq = *(concRequest*) hBuf_.src();
	
	try
	{
		boost::asio::read(socket_, boost::asio::buffer( source_.nsrc(rq.receiver_id_size) ,rq.receiver_id_size));
	
		std::string receiver(source_.to_string());
		Qr::insert_connection(Q::DB, user_name, receiver , 0);
	
		nmap_notify(*N, receiver , checkListIndex::connectionReq);
		BOOST_LOG_TRIVIAL(trace) << "Connection Inserted";
	}

	catch (boost::system::error_code& err)
	{
		BOOST_LOG_TRIVIAL(info) << err.message();
	}
	task_end(err,0);

}

void session_no_ssl::send_Okay(const boost::system::error_code& err)
{
	char A;
	try
	{
		boost::asio::async_write(socket_, boost::asio::buffer(&A, 1),
			boost::bind(&session_no_ssl::task_end, this , boost::asio::placeholders::error, 0));
	}

	catch(boost::system::error_code& err)
	{
		BOOST_LOG_TRIVIAL(info) << err.message();
		error_handle(err);
	}
	
}

void session_no_ssl::response_act_send_msg(const boost::system::error_code& err, pBuffer& p)
{
	try
	{
		msReq* req = (msReq*)p.src();
		std::string text;
		
		boost::asio::read(socket_, boost::asio::buffer(source_.nsrc(req->sz), req->sz));
		Qr::insert_message(Q::DB , *req , source_.to_string() );

		auto mList = Qr::get_dining_members(Q::DB, req->table_key);
		nmap_notify_with_list( *N , *mList, checkListIndex::messageArr );
	
	}

	catch (boost::system::error_code& err)
	{
		BOOST_LOG_TRIVIAL(info) << err.message();
		error_handle(err);

	}

	task_end(err, 0);

}



void session_no_ssl::read_packet(const boost::system::error_code& err, void* cont)
{
	try
	{
		if (!pc.empty())
		{
			pchd h = pc.front();

			if (h.actionOrCommand == AC::Action)
				Action(err);

			else if (h.actionOrCommand == 1)
				Command(err);
		}


	}

	catch ( const boost::system::error_code& err)
	{
		BOOST_LOG_TRIVIAL(trace) << err.message();
		disconnect(err);
	}
}


void session_no_ssl::read_handle(const boost::system::error_code& err)
{
	try
	{
		pchd h = pc.front();
		void* argv = 0;

		if (h.actionOrCommand == AC::Command)
		{
			Command(err);
			return;
		}

		else
		{
			Action(err);
		}
	}

	catch (const boost::system::error_code& err)
	{
		BOOST_LOG_TRIVIAL(trace) << err.message();
		disconnect(err);
	}
}

void session_no_ssl::disconnect(const boost::system::error_code& err)
{

	socket_.close();

}


void session_no_ssl::request_login()
{

}

void session_no_ssl::rget_new_message(const boost::system::error_code& err)
{
	pchd h = {};

	h.actionOrCommand = AC::Command;

	h.ack = 1;
	h.comd = eCommand::GetMsg;
	int lastCheck_ = checkManager->at(checkListIndex::messageArr);

	h.count = Qr::get_new_message(Q::DB, source_ , user_name, lastCheck_);

	try
	{
		boost::asio::write(socket_ , boost::asio::buffer( &h , sizeof(pchd)));
		boost::asio::async_write(socket_ , boost::asio::buffer(source_.src() , source_.written_size() ),
			boost::bind(&session_no_ssl::task_end, this, boost::asio::placeholders::error, 0));
	}

	catch (boost::system::error_code& err)
	{
		error_handle(err);
	}

	checkManager->at(checkListIndex::messageArr) = lastCheck_;
	task_end(err, 0);
}


bool session_no_ssl::session_init(const boost::system::error_code& err, std::string username)
{
	if (err)
		error_handle(err);

	checkManager = Qr::get_check_list(Q::DB, username);
	user_name = username;

	this->nf = new boost::array<nof, 10>();
	auto resultIter = N->insert( user_name , nf );
	
	initialized = true;
	return true;
}


void session_no_ssl::error_handle(const boost::system::error_code& err)
{

	if (
			err == boost::asio::error::eof ||
			err == boost::asio::error::connection_reset
		)
	{
		BOOST_LOG_TRIVIAL(trace) << "Connction Aborted";

		socket_.close();
		initialized = false;
	}

}

void session_no_ssl::response_req_get_new_ivt(const boost::system::error_code& err)
{
	pchd h = pc.front();
	h.ack = 1;
	h.actionOrCommand = 1;
	
	if (err)
		error_handle(err);

	pBuffer pb(256);

	pb.reserve_with_tag( sizeof(pchd) , "Header");
	int count = 0;

	int lastCheck_ = checkManager->at(checkListIndex::tableIvt);
	h.count = Qr::get_new_ivt_req(Q::DB , user_name , pb , lastCheck_);
	
	pb.allocate_with_tag("Header", &h , sizeof(pchd));

	try
	{
		boost::asio::async_write(socket_, boost::asio::buffer(pb.src(), pb.written_size()),
			boost::bind(&session_no_ssl::task_end, this, boost::asio::placeholders::error, 0));
	}

	catch (const boost::system::error_code& err)
	{
		error_handle(err);
	}

}


void session_no_ssl::response_req_get_new_conc(const boost::system::error_code& err)
{
	pchd h = {};
	h.actionOrCommand = 1;
	h.comd = eCommand::GetConnection;

	if (err)
		error_handle(err);

	pBuffer p(256);
	p.reserve_with_tag(sizeof(pchd), "Header");

	int lastCheck_ = checkManager->at(checkListIndex::connectionReq);
	h.count = Qr::get_new_connection_req(Q::DB, user_name , p , lastCheck_);
	
	memcpy(p.get_src_reserved_with_tag("Header"), &h , sizeof(pchd));

	try
	{
		boost::asio::async_write(socket_ , boost::asio::buffer(p.src() , p.written_size() ),
			boost::bind(&session_no_ssl::task_end, this , boost::asio::placeholders::error, 0));
	}

	catch (boost::system::error_code& err)
	{
		error_handle(err);
	}
}

void session_no_ssl::recommand( constBoostErr& err , int e )
{



}


void session_no_ssl::response_act_invite(const boost::system::error_code& err, pBuffer& h)
{
	if (err)
		error_handle(err);

	ivtReq* req = (ivtReq*)h.src();
	pBuffer target_getter(req->target_id_size);

	boost::asio::read(socket_, boost::asio::buffer(target_getter.src(), req->target_id_size));
	Qr::insert_invite(Q::DB, target_getter.to_string() , user_name, req->table_key);

	nmap_notify(*N, target_getter.to_string() , checkListIndex::tableIvt);
	task_end(err,0);
}

void session_no_ssl::reaction(const boost::system::error_code& err , int e )
{

}

void session_no_ssl::response_req_get_table_mine(const boost::system::error_code& err)
{

}
