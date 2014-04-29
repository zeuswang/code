//  Copyright (c) 2007-2008 Facebook
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.
//
// See accompanying file LICENSE or visit the Scribe site at:
// http://developers.facebook.com/scribe/
//
// @author Bobby Johnson
// @author James Wang
// @author Jason Sobel
// @author Avinash Lakshman
// @author Anthony Giardullo
#include "common.h"
#include "scribe_handler.h"
#include <utils/implement_base.h>

using namespace apache::thrift;
using namespace apache::thrift::concurrency;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;

using namespace facebook::fb303;
using namespace facebook;

using namespace scribe::thrift;
using namespace std;

using boost::shared_ptr;

//shared_ptr<scribeHandler> g_Handler;

void print_usage(const char* program_name) {
  cout << "Usage: " << program_name << " [-p port] [-c config_file]" << endl;
}

scribeHandler::scribeHandler(unsigned long int server_port)
  : FacebookBase("Scribe"),
    port(server_port),
    status(STARTING),
    statusDetails("initial state") {
}

scribeHandler::~scribeHandler() {
}

// Returns the handler status, but overwrites it with WARNING if it's
// ALIVE and at least one store has a nonempty status.
fb_status scribeHandler::getStatus() {
  return ALIVE;
}

void scribeHandler::setStatus(fb_status new_status) {
  LOG_OPER("STATUS: %s", statusAsString(new_status));
  Guard status_monitor(statusLock);
  status = new_status;
}

// Returns the handler status details if non-empty,
// otherwise the first non-empty store status found
void scribeHandler::getStatusDetails(std::string& _return) {
  Guard status_monitor(statusLock);

  _return = statusDetails;
  return;
}

void scribeHandler::setStatusDetails(const string& new_status_details) {
  LOG_OPER("STATUS: %s", new_status_details.c_str());
  Guard status_monitor(statusLock);
  statusDetails = new_status_details;
}

const char* scribeHandler::statusAsString(fb_status status) {
  switch (status) {
  case DEAD:
    return "DEAD";
  case STARTING:
    return "STARTING";
  case ALIVE:
    return "ALIVE";
  case STOPPING:
    return "STOPPING";
  case STOPPED:
    return "STOPPED";
  case WARNING:
    return "WARNING";
  default:
    return "unknown status code";
  }
}



ResultCode scribeHandler::Log(const vector<LogEntry>&  messages) {
  ResultCode result = TRY_LATER;
  bool busy=false;
  int ret=0;

  if(status == STOPPING) {
    result = TRY_LATER;
    goto end;
  }
  for (vector<LogEntry>::const_iterator msg_iter = messages.begin();
       msg_iter != messages.end();
       ++msg_iter) {
		//ReceiverTask::instance()->GetLog(msg_iter->message);
	// disallow blank category from the start
    if (category_.empty()){
		std::string log;
		ret=impl_->Run(const_cast<std::string*>(&(msg_iter->message)));
		if (ret<0)
			busy=true;
    }else {
	    if ((*msg_iter).category == category_) {
			//HandleTask::instance()->Put(msg_iter->message);
			
			ret=impl_->Run(const_cast<std::string*>(&(msg_iter->message)));
			if (ret<0)
				busy=true;
	    }
    }
	
  }
  if (busy){
	result = TRY_LATER;
  }else {
	result = OK;
  }

end:
  return result;
}


void scribeHandler::shutdown() {
  // calling stop to allow thrift to clean up client states and exit
  exit(0);
}

void scribeHandler::reinitialize() {

  // reinitialize() will re-read the config file and re-configure the stores.
  // This is done without shutting down the Thrift server, so this will not
  // reconfigure any server settings such as port number.
  LOG_OPER("reinitializing");
  initialize();
}

void scribeHandler::initialize() {

  // This clears out the error state, grep for setStatus below for details
	setStatus(STARTING);
	setStatusDetails("");
	setStatus(ALIVE);
}



