#include "Node.h"



Node::Node()
{
}


Node::~Node()
{
}

void Node::setIP(std::string IP_)
{
	IP = IP_;
}

void Node::setPort(unsigned int port_)
{
	port = port_;
}

void Node::setID(unsigned int ID_)
{
	ID = ID_;
}

void Node::setState(state_n state_)
{
	state = state_;
}

void Node::setErrorType(errorType_n errorType_)
{
	errorType = errorType_;
}

void Node::setErrorMessage(std::string errorMessage_)
{
	errorMessage = errorMessage_;
}

std::string Node::getIP(void)
{
	return IP;
}

unsigned int Node::getPort(void)
{
	return port;
}

errorType_n Node::getErrorType(void)
{
	return errorType;
}

std::string Node::getErrorMessage(void)
{
	return errorMessage;
}

unsigned int Node::getID(void)
{
	return ID;
}

state_n Node::getState(void)
{
	return state;
}

std::map <unsigned int, Neighbour> Node::getNeighbours(void)
{
	return neighbours;
}

std::vector<std::string> Node::getFilters(void)
{
	return FilterArray;
}
