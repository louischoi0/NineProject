OBJECTS = param.o query.o session.o nof.o pr9_s.o server.o 
STDAFX = -include ../stdafx.h
VIEWH = -H
TARGET = PrNine 
INBOOST = -I ~/boost_1_64_0 -lboost_regex -lboost_date_time -lboost_iostreams -lboost_thread -lpthread -lboost_chrono -lboost_context -lboost_system -lboost_log -lboost_locale -lsqlite3
STDVERSION = -std=c++11
PREFIX = -DBOOST_LOG_DYN_LINK -g -O0
OPTIONS =  

$(TARGET) : $(OBJECTS)
	g++ $(STDVERSION) $(VIEWH) $(STDAFX) $(PREFIX) -o $(TARGET) $(OBJECTS) $(INBOOST) $(OPTIONS)

pr9_s.o : stdafx.h server.h session.h hqery.h pack.hpp pr9_s.cpp
	g++ $(STDVERSION) $(STDAFX) $(PREFIX) -c pr9_s.cpp $(VIEWH) $(INBOOST) $(OPTIONS)
param.o : stdafx.h param.h param.cpp
	g++ $(STDVERSION) $(STDAFX) $(PREFIX) -c param.cpp $(VIEWH) $(INBOOST)  $(OPTIONS)
query.o : stdafx.h query.cpp
	g++ $(STDVERSION) $(STDAFX) $(PREFIX) -c query.cpp $(VIEWH) $(INBOOST)  $(OPTIONS)
server.o : stdafx.h session.h server.h
	g++ $(STDVERSION) $(STDAFX) $(PREFIX) -c server.cpp $(VIEWH) $(INBOOST)  $(OPTIONS)
session.o : stdafx.h session.h session.cpp
	g++ $(STDVERSION) $(STDAFX) $(PREFIX) -c session.cpp $(VIEWH) $(INBOOST) $(OPTIONS)
nof.o : stdafx.h nof.h
	g++ $(STDVERSION) $(STDAFX) $(PREFIX) -c nof.cpp $(VIEWH) $(INBOOST)$(OPTIONS) 
