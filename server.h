#ifndef SERVER
#define SERVER

#include "session.h"
#include "stdafx.h"


//typedef boost::shared_ptr<session> session_ptr;
typedef session_no_ssl session;

class server_base
{
public :
	server_base(boost::asio::io_service& io_service, unsigned short port) :
		io_service_(io_service),
		acceptor_(io_service,
			boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
		db(Q::DB)

	{

	}

	virtual void handle_accept(session* new_session,
		const boost::system::error_code& error)
	{

	}
	
protected:
	boost::asio::io_service& io_service_;
	tcp::acceptor acceptor_;
	sqlite3* db;

};
#define DEFAULT_SERVER_THREAD 400
#define DEFAULT_SERVER_ACCEPT_THREADS 80
#define DEFAULT_SERVER_BACKGROUND_TASK_THREADS 20
class server_no_ssl : server_base
{
public :
	server_no_ssl(boost::asio::io_service& io, unsigned short port) :
		server_base(io, port),
		socket_(io),
		io_(io)
	{
		set_threads();
		io_.post( boost::bind(&server_no_ssl::server_run , this ));
	}
	void set_threads()
	{
		threadPool_ = std::list<boost::thread*>( DEFAULT_SERVER_THREAD, new  boost::thread(boost::bind( &io_service::run, &io_)));
		threadPool_.merge( std::list<boost::thread*>( DEFAULT_SERVER_BACKGROUND_TASK_THREADS ,new boost::thread(boost::bind(&server_no_ssl::server_back_ground_task,this))));

	}
		
	void handle_accept(session_no_ssl* session, const boost::system::error_code& err)
	{
		sessionPool_.push_back(session);
		io_.post(boost::bind(&session_no_ssl::run , session , err));

		server_run();
	}


	void server_back_ground_task()
	{
		client_check_mail_box();

	}

	void server_run()
	{
		session_no_ssl *csession;

		if (!reusablePool_.empty())
		{
			csession = reusablePool_.front();
			reusablePool_.pop();
		}

		else
			csession = new session_no_ssl(io_service_, &N);

		acceptor_.async_accept(csession->socket(),
			boost::bind( &server_no_ssl::handle_accept, this ,csession, boost::asio::placeholders::error ));
	
	}
	
	void client_check_mail_box()
	{

		while (true)
		{
			auto iter = sessionPool_.begin();

			for (; iter != sessionPool_.end(); iter++)
				io_.post(boost::bind(&session_no_ssl::check_mail_box, *iter));
		}
	}

	void add_thread_to_io()
	{
		boost::thread* trd = new  boost::thread([&]{ io_.run(); });
		threadPool_.push_back(trd);
	}

	static void insert_session_into_reusable_list(session_no_ssl* session)
	{
		reusablePool_.push(session);
	}

	static void notify_abort_connection(std::list<session_no_ssl*>::iterator session_iter)
	{
		
		insert_session_into_reusable_list(*session_iter);
		sessionPool_.erase(session_iter);
	
	}

	static void insert_active_sesssion_into_active_pool(session_no_ssl* session)
	{

	}

private:
	nsocket socket_;
	static nMap N;
	
	static std::list<boost::thread*> threadPool_;
	static std::list<session_no_ssl*> sessionPool_;
	static std::queue<session_no_ssl*> reusablePool_;

	static std::list<session_no_ssl*> activeSessionPool_;

	boost::asio::io_service& io_;
	boost::thread* server_task_thread;
	int interval_;

};

#endif
