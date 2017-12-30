#pragma once
#pragma comment(lib, "lib/sqlite3.lib")

#include "stdafx.h"
#include "hqery.h"
#include "param.h"

using namespace std;
using namespace boost;
using namespace boost::container;

class nof : public Param
{	
	short type;
	
	int rowid_checked_last;
	bool _new;

	std::string _tag;

public :
	nof() : _new(false)
	{};

	void resee() { };
	bool get_new() { return _new; };

	inline void task_end() { _new = false; }
	
	void renew() { _new = true; BOOST_LOG_TRIVIAL(trace); };

	void set_params(defaultType a0, defaultType a1, defaultType a2, defaultType a3)
		{ 
			dType d[4] = { a0, a1, a2, a3 };
				set(4, d); 
		};
	
	void set_params(defaultType a0, defaultType a1)
	{
		dType d[2] = { a0, a1 };
		set(2, d);
	};
	void set_params(defaultType a0, defaultType a1, defaultType a2)
	{
		dType d[3] = { a0, a1, a2 };
		set(3, d);
	};

	void set_params(defaultType a0)
	{
		set(1, &a0);
	};

	void set_tag(std::string s)
	{
		_tag = s;
	}


private :

};

class notifier
{

public:
	notifier(int id) {};
	notifier();

private:

	int uid;
	boost::array<nof, 10> notification;
	bool new_;

public:
	bool any();
	
	void task_end(int i) { notification[i].task_end(); };
	bool get_notification(int i) { return notification[i].get_new(); };

	nof* at(int i) { return &notification[i]; };

};

typedef boost::array<nof, 10> NotificationArr;

class nMap
{
public :
	nMap() {};

	boost::array<nof, 10>* insert(std::string id, NotificationArr* notification) {
		auto res = Map.insert( std::pair<std::string, boost::array<nof, 10>* >(id, notification ));
		return res.first->second;
	};

	boost::optional<boost::array<nof, 10>*> get_notifier(std::string id)
	{
		auto res = Map.find(id);
		
		if (res == Map.end())
			return boost::none;

		else
			return Map.find(id)->second;
	};

	std::map< std::string, NotificationArr* >& get_map() { return Map; };

private:
	std::map< std::string, boost::array<nof, 10>* > Map;

};



