#include "SIONotificationHandler.h"
#include "Poco/Observer.h"
#include "SIONotifications.h"
#include "Poco/NotificationCenter.h"
#include "Poco/WindowsConsoleChannel.h"

#include "Poco/JSON/Parser.h"
#include "Poco/JSON/DefaultHandler.h"

#include "SIOEventRegistry.h"

using Poco::WindowsConsoleChannel;
using Poco::Observer;
using Poco::JSON::Parser;
using Poco::JSON::DefaultHandler;
using Poco::Dynamic::Var;
using Poco::JSON::Array;
using Poco::JSON::Object;

SIONotificationHandler::SIONotificationHandler(void)
{
}

SIONotificationHandler::SIONotificationHandler(NotificationCenter* nc)
{
	_nCenter = nc;
	registerCallbacks(_nCenter);

	_logger = &(Logger::get("SIOClientLog"));
	_logger->setChannel(new WindowsConsoleChannel());
}

SIONotificationHandler::~SIONotificationHandler(void)
{
	_nCenter->removeObserver(
		Observer<SIONotificationHandler, SIOMessage>(*this, &SIONotificationHandler::handleMessage)
		);
	_nCenter->removeObserver(
		Observer<SIONotificationHandler, SIOJSONMessage>(*this, &SIONotificationHandler::handleJSONMessage)
		);
	_nCenter->removeObserver(
		Observer<SIONotificationHandler, SIOEvent>(*this, &SIONotificationHandler::handleEvent)
		);
}

void SIONotificationHandler::handleMessage(SIOMessage* pNf)
{
	_logger->information("handling message, message received: %s",pNf->getMsg());
	pNf->release();
}

void SIONotificationHandler::handleJSONMessage(SIOJSONMessage* pNf)
{
	_logger->information("handling JSON message");
	pNf->release();
}

void SIONotificationHandler::handleEvent(SIOEvent* pNf)
{
	_logger->information("handling Event");
	_logger->information("data: %s", pNf->_data);

	Parser parser;
	DefaultHandler handler;

	parser.setHandler(&handler);
	parser.parse(pNf->_data);

	Var result = handler.result();
	Object::Ptr object = result.extract<Object::Ptr>();
	Var temp = object->get("name");

	std::string eventName = temp.convert<std::string>();
	
	Array::Ptr arr = object->getArray("args");
	Object::Ptr args = arr->getObject(0);

	SIOEventRegistry::sharedInstance()->fireEvent(pNf->_client, eventName.c_str(), args);

	pNf->release();
}

void SIONotificationHandler::registerCallbacks(NotificationCenter* nc)
{
	_nCenter = nc;

	_nCenter->addObserver(
		Observer<SIONotificationHandler, SIOMessage>(*this, &SIONotificationHandler::handleMessage)
		);
	_nCenter->addObserver(
		Observer<SIONotificationHandler, SIOJSONMessage>(*this, &SIONotificationHandler::handleJSONMessage)
		);
	_nCenter->addObserver(
		Observer<SIONotificationHandler, SIOEvent>(*this, &SIONotificationHandler::handleEvent)
		);
}

void SIONotificationHandler::setNCenter(NotificationCenter* nc)
{
	_nCenter = nc;
	registerCallbacks(_nCenter);
}