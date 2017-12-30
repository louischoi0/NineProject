// pr9_s.cpp: 콘솔 응용 프로그램의 진입점을 정의합니다.
//
#include "stdafx.h"
#include "server.h"
#include "session.h"
#include "hqery.h"
	
using namespace std;

nMap server_no_ssl::N = nMap();

std::list<boost::thread*> server_no_ssl::threadPool_ = std::list<boost::thread*>();
std::list<session_no_ssl*> server_no_ssl::activeSessionPool_ = std::list<session_no_ssl*>();
std::list<session_no_ssl*> server_no_ssl::sessionPool_ = std::list<session_no_ssl*>();
std::queue<session_no_ssl*> server_no_ssl::reusablePool_ = std::queue<session_no_ssl*>();

int main(int argc, char* argv[])
{	

	pBuffer p(256);
	char* err;
	int i = 0;
	boost::asio::io_service io_service;
	
	boost::asio::io_service::work w_(io_service);

	boost::thread server_work0([&] { 	io_service.run();  });
	boost::thread server_work1([&] { 	io_service.run();  });
	boost::thread server_work2([&] { 	io_service.run();  });
	boost::thread server_work3([&] { 	io_service.run();  });

	server_no_ssl ss(io_service, 8899);

	std::vector<std::string> cont;
	using namespace std; // For atoi.
	
	boost::thread command_work1([&] {

		std::vector<std::string> cont;
		while (1)
		{
			char* command;

			BOOST_LOG_TRIVIAL(trace) << "Server Command >";

			std::string res;
			std::getline(std::cin, res);

			boost::split(cont, res, boost::is_any_of(" "));

			auto iter = cont.begin();
		
			if( *iter == "check"	)
				ss.client_check_mail_box();
			else if (*iter == "exit")
				exit(0);
			BOOST_LOG_TRIVIAL(trace) << "\n";

		}
	
	});
	
	

	while (true)
	{

	}

	return 0;

}
