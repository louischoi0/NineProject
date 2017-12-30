#ifndef CHAT_MESSAGE_HPP
#define CHAT_MESSAGE_HPP

#include "stdafx.h"
#include "param.h"

typedef boost::asio::ip::tcp::socket nsocket;

typedef struct pc_hd
{
	short sz;

	short actionOrCommand;
	short comd;

	int suk;
	int ack;

	int count;

	char* pbuf;

}pchd;

typedef boost::array<char, 1024> pB;

class pBuffer : public Param
{

public:
	pBuffer() : _size(0), _data(0), next_input_point(0), reserved_index(0),
		Param(defaultType::Int, defaultType::Int, defaultType::Int, defaultType::Int)
	{

	};
	pBuffer(int size) : _size(size),  next_input_point(0), reserved_index(0),
		Param(defaultType::Int, defaultType::Int, defaultType::Int, defaultType::Int)
	{
		a_src(size);
	}

	pBuffer(std::string s) : _size(s.size()), next_input_point(0), reserved_index(0),
		Param(defaultType::Int, defaultType::Int, defaultType::Int, defaultType::Int)
	{
		init(_size);
		memcpy(_data.get() , s.c_str(), _size);

	}


	~pBuffer()
	{
		_data.reset();
	
	}

	void reserve_front(int size) {};
	void init_record() { next_input_point = 0; } ;
	int init(int size) { _data.reset(new char[size]); _size = size; return size; };
	boost::asio::mutable_buffers_1 wscr(int size ) 
		{ 	int now = next_input_point;
			next_input_point += size; 
			return boost::asio::buffer( now + _data.get() , size  );
		}
	char* src() { return _data.get(); };
	char* nsrc(int size) { _size = size; next_input_point = size; return _data.get();};
	char* a_src(int size) { init(size); return _data.get(); };
	char* src_inc_alloc(int size) { next_input_point += size; return _data.get(); };
	char* src_inc_alloc_and_tag(int size, std::string s) 
	{
		if (reserved_index > 4)
			return 0;

		set_args(reserved_index, s);

		set_value<int>( s , next_input_point);

		next_input_point += size;
		reserved_index++;

		return get_src_reserved_with_tag(s);
	};
			
	int size() { return _size; };

	pBuffer& operator=(std::string s)
	{
		init(s.size());
		memcpy( _data.get() , s.c_str(), _size);
	}

	std::string to_string()
	{
		return std::string( _data.get() , next_input_point);
	}

	void reserve_with_tag(int size, std::string tag_name)
	{
		if (reserved_index > 4)
			return;
		
		set_args( reserved_index, tag_name );
	
		set_value<int>(tag_name,  next_input_point);
		
		next_input_point += size;
		reserved_index++;
	}

	void allocate_some(void* src , int size)
	{
		if ( next_input_point + size > _size )
		{
			char* new_address = new char[_size * 2];
			memcpy( new_address , _data.get() , _size );

			_size *= 2;
			_data.reset(new_address);
		}
		
		memcpy(_data.get() + next_input_point , src, size);
		next_input_point += size;
	
	
	}

	void allocate_with_tag(std::string s, void* src , int size)
	{
		memcpy(get_src_reserved_with_tag(s), src, size);
	
	}

	char* get_src_reserved_with_tag(std::string s)
	{
		return get_value<int>(s) + _data.get();
	}

	inline int written_size() { return next_input_point; };

private:
	int _size;
	int next_input_point;

	int reserved_index;
	boost::shared_ptr<char> _data;

};

typedef std::pair< pBuffer*, int*> queryContainer;

class Packer : public boost::enable_shared_from_this<Packer> 
{

public:
	enum { header_length = sizeof(short) + sizeof(int) * 3 };
	enum { max_body_length = 512 };
	
	Packer()
	{

	}

	Packer(pB* p)
		: 
		body_length_(0),
		complete(false),
		remain(0),
		current(0),
		iData_(p)
	{

	}


	bool decode_hader(const boost::system::error_code& err)
	{
		if (!err)
		{
			pchd hh = {};
			memcpy(&hh, data(), sizeof(pchd));
			
			if (hh.sz < 0)
				return false;
			
			if(!hQue.empty()) 			
			{
				pchd& pp =  hQue.front();
				if(pp.comd == hh.comd)
					return true;

			}
			else
			{
				int size = hh.sz;

				if (size > 1024)
					return false;

				hQue.push_back(hh);
			
				return true;
			}

		}
	
		else
		{
			std::cerr << err.message() << std::endl;

		}

	
	}

	void read_element(const boost::system::error_code& err, nsocket& s, pBuffer& element, int size)
	{
		try
		{
			boost::asio::read(s, boost::asio::buffer(element.a_src(size), size));

		}

		catch (const boost::system::error_code& err)
		{


		}


	}


	template<class H>
	void read_element_with_pair
	(const boost::system::error_code& err , nsocket& s, H& header , pBuffer& element, int header_size )
	{
		try
		{
			boost::asio::read(s, boost::asio::buffer(&header , header_size));

			int element_size = header.sz;

			BOOST_LOG_TRIVIAL(trace) << "Packet Size :" << element_size;

			if (element_size > 0)
			{
				s.read_some(boost::asio::buffer( element.a_src(element_size) , element_size));
			}
		
		}

		catch (const boost::system::error_code& err)
		{
			BOOST_LOG_TRIVIAL(info) << err.message();
		}

	}

	template< class H >
	std::list<std::pair< H , pBuffer > >* decode_any_case(const boost::system::error_code& err , nsocket* s)
	{
		try
		{
			auto res = new std::list<std::pair< H , pBuffer >>();
			boost::shared_ptr<std::list<std::pair< H , pBuffer > > > p;

			p.reset(res);

			pchd h = front();
		
			int header_size = h.sz;
			int serial_size = h.count;

			for (int i = 0; i < h.count; i++)
			{
				res->push_back(std::pair< H , pBuffer >());
				auto it = res->back();
				
				read_element_with_pair(err, s , it->first, it->second , h.sz );
			
			}
				
			return p;
		}

		catch (const boost::system::error_code& err)
		{
			BOOST_LOG_TRIVIAL(info) << err.message();
			return 0;
		}


	}

	void decode(const boost::system::error_code& err)
	{
		if (!err)
		{
			pchd h = hQue.front();

			memcpy(h.pbuf, (void*)body(), h.sz);

			std::cout << "push complete" << std::endl;
		}

		else
		{
			std::cerr << err.message() << std::endl;

		}

	}

	bool completed() { return complete; };
	pchd get_hader() { return std::move(hd); };
	
	char* data() { return iData_->data(); };
	char* body() { return iData_->data() + sizeof(pchd); };

	int fsize() { return hQue.front().sz; };
	void push_back(const pchd& h) { hQue.push_back(h); };

private:

	pB* iData_;
	pB* nData_;

	boost::container::deque<pchd> hQue;
	boost::container::deque<std::string> sQue;

	std::size_t body_length_;

	int remain;
	int current;


	pchd hd;

	bool complete;

	public:
		void readBytes(const boost::system::error_code& error ,	size_t bytes_transferred);

		int end;

		char seg[1024];

		inline void pop()
		{
			hQue.pop_front();
		};

		bool empty() { return hQue.empty(); };
		pchd front() { return hQue.front(); };


};

#endif // CHAT_MESSAGE_HPP
