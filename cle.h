#pragma once

#include "stdafx.h"
#include "nof.h"
#include "Pack.hpp"

#define max_length 1024

using namespace std;

using namespace boost::asio;
using namespace boost::asio::ip;

typedef struct boost::asio::ip::tcp::socket nsocket;

class client_base : public boost::enable_shared_from_this<client_base>
{
public : 
	client_base() :
	nf(NULL), init(false)
	{};

	client_base(notifier* n) :
		nf(n) , init(false)
	{};


	void set_notifier(notifier* n) { nf = n; init = true; };
	bool get_init() { return init; };


protected:
	bool init;
	notifier *nf;
	
};

class client_no_ssl : public client_base
{
public:
	client_no_ssl(boost::asio::io_service& io, boost::asio::ip::tcp::resolver::iterator endpoint_iterator )
		: 
			socket_(io),
			i(0)
	{
		BOOST_LOG_TRIVIAL(trace) << "Client Construct";
		start(endpoint_iterator);
		
	}

	void start(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
	{

		BOOST_LOG_TRIVIAL(trace) << "Client Connection request";

		boost::asio::async_connect(socket_, endpoint_iterator,
			boost::bind(&client_no_ssl::connect, this,
				boost::asio::placeholders::error));

	}

	void routine( const boost::system::error_code& err )
	{
		if (!err)
		{
			while (false)
			{
				if (nf->get_notification(0))
				{
					nof* n = nf->at(0);
					std::string pw = n->get_value<std::string>("PW");
					std::string id = n->get_value<std::string>("ID");

					req_login(err, id , pw);

					n->task_end();
				}

				else if (nf->get_notification(1))
				{
					nof* n = nf->at(1);

					int table_key = n->get_value<int>("TableKey");
					int sender_key = n->get_value<int>("Sender");
					std::string text = n->get_value<std::string>("Sender");

				}


				boost::this_thread::sleep(boost::posix_time::millisec(2000));
				
			}

		}

		else
		{
			std::cerr << err.message() << std::endl;
		}


	}

	


	void req_send_msg(const boost::system::error_code& err, int tableKey, int senderKey, std::string text)
	{
	}

	void req_send_msg_handle( const boost::system::error_code& err , std::string text)
	{
		boost::asio::async_write(socket_, boost::asio::buffer(text.c_str(), text.size() ),
			boost::bind( &client_no_ssl::routine, this, boost::asio::placeholders::error));
	
	}

	void req_login(const boost::system::error_code&, std::string id , std::string pw)
	{
		pchd h = {};
	//	h.comd = eCommand::;
		
		rqlog r = {};
		r.sizeof_id = id.size();
		r.sizeof_pw = pw.size();

		boost::asio::async_write(socket_ , boost::asio::buffer(&h , sizeof(pchd)),
			boost::bind(&client_no_ssl::req_login_handle, this , boost::asio::placeholders::error, r, id , pw ));
	
	
	}

	void req_login_handle(const boost::system::error_code& err, rqlog r, std::string id , std::string pw)
	{
		char buf[512];
		char* buff = buf;
		int size = sizeof(rqlog) + id.size() + pw.size();
		
		memcpy(buff, &r, sizeof(rqlog));
		buff += sizeof(rqlog);

		memcpy(buff, id.c_str(), id.size() );
		buff += id.size();

		memcpy(buff, pw.c_str(), pw.size());
		buff += pw.size();

		boost::asio::write(socket_, boost::asio::buffer(buf, size) );
	
	}

	void connect(const boost::system::error_code& err)
	{	
		BOOST_LOG_TRIVIAL(trace) << "Client Connection handle";
		
		if (!err)
		{
			routine(err);
		}

		else
		{
			std::cerr << err.message() << std::endl;

		}


	}
	
	void write_haeder_handle(const boost::system::error_code& err, std::string str)
	{

		pchd h = {};

		h.sz = str.size();
		h.ack = 1;

		if (!err)
		{

			if (i > 5)
			{
				i++;

				boost::asio::async_read(socket_, boost::asio::buffer(&h, sizeof(h)),
					boost::bind(&client_no_ssl::write_handle, this, boost::asio::placeholders::error,
						str));

				std::cout << " send haeder of size : " << h.sz << std::endl;

			}

			else
				boost::asio::async_write(socket_, boost::asio::buffer(&h, sizeof(h)),
					boost::bind(&client_no_ssl::write_handle, this, boost::asio::placeholders::error,
						str));
		}

		else
		{
			std::cerr << err.message() << std::endl;

		}

	}

	void write_handle(const boost::system::error_code& err, std::string str)
	{
		if (!err)
		{
			i++;

			std::string s = "send atttt " + std::to_string(i);

			boost::asio::async_write(socket_, boost::asio::buffer(str.c_str(), str.size()),
				boost::bind(&client_no_ssl::write_haeder_handle, this, boost::asio::placeholders::error, s));

			std::cout << " send string : " << str << std::endl;

		}

		else
		{
			std::cerr << err.message() << std::endl;

		}


	}

private:
	nsocket socket_;
	int i;

	boost::thread 
		
		
		
		Thread;



};


class client
{
public:
	client(boost::asio::io_service& io_service,
		boost::asio::ssl::context& context,
		boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
		: socket_(io_service, context)
	{

		socket_.set_verify_callback(
			boost::bind(&client::verify_certificate, this, _1, _2));

		boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
			boost::bind(&client::connect, this,
				boost::asio::placeholders::error));

	}
	bool verify_certificate(bool preverified,
		boost::asio::ssl::verify_context& ctx)
	{
		char subject_name[256];
		X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
		X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);

		std::cout << "Verifying " << subject_name << "\n";

		return preverified;
	}

	void connect(const boost::system::error_code& error)
	{
		if (!error)
		{	/*
			socket_.async_handshake(boost::asio::ssl::stream_base::client,
			boost::bind(&client::shandle, this,
			boost::asio::placeholders::error));
			*/

			pchd h = {};
			h.sz = 919;
			socket_.async_write_some(boost::asio::buffer(&h, sizeof(h))
				, boost::bind(&client::handle_write, this, boost::asio::placeholders::error));

		}

		else
		{
			std::cout << "Connect failed: " << error.message() << "\n";
		}

	}
	void handle_write(const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "send bytes " << std::endl;
		}

		else
		{
			std::cout << "Write failed: " << error.message() << "\n";
		}
	
	}
	void shandle(const boost::system::error_code& error)
	{
		if (!error)
		{
			std::cout << "Enter message: ";
			std::cin.getline(request_, max_length);
			size_t request_length = strlen(request_);

			boost::asio::async_write(socket_,
				boost::asio::buffer(request_, request_length),
				boost::bind(&client::handle_read, this,
					boost::asio::placeholders::error,
					boost::asio::placeholders::bytes_transferred));
		}
		else
		{
			std::cout << "Handshake failed: " << error.message() << "\n";
		}

		std::cout << "complete" << std::endl;
	}

	void handle_read(const boost::system::error_code& err, int transferred)
	{
		std::cout << "read handle" << std::endl;
	}
private:
	boost::asio::ssl::stream<nsocket> socket_;

	char request_[max_length];
	char reply_[max_length];


};
