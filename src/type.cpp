//
//  type.cpp
//  hlsyn
//
//  Created by Jason Tran on 11/25/17.
//

#include "type.h"

string type::getName()
{
    return name;
}

void type::setName(string name)
{
    this->name = name;
}

int type::getDataWidth()
{
    return dataWidth;
}

void type::setDataWidth(int dataWidth)
{
    this->dataWidth = dataWidth;
}

bool type::getUsigned()
{
    return usigned;
}

void type::setUsigned(bool usigned)
{
    this->usigned = usigned;
}

vector<node*> type::getOutgoing()
{
    return outgoing;
}

void type::setOutgoing(vector<node*> outgoing)
{
    this->outgoing = outgoing;
}

vector<node*> type::getIncoming()
{
    return incoming;
}

void type::setIncoming(vector<node*> incoming)
{
    this->incoming = incoming;
}

void type::appendOutgoing(node* outgoing)
{
    this->outgoing.push_back(outgoing);
}

void type::appendIncoming(node* incoming)
{
    this->incoming.push_back(incoming);
}


