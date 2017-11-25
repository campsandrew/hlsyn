//
//  type.h
//  hlsyn_project
//
//  Created by Jason Tran on 11/25/17.
//

#ifndef DATATYPE_H
#define DATATYPE_H

#include<string>
#include<vector>
#include<cstring>
#include<algorithm>

using namespace std;

class node;

class type {
private:
    string name;
    int dataWidth;
    bool usigned; // 1 unsigned, 0 signed
    vector<node*> incoming; // nodes that current node originates from
    vector<node*> outgoing; // nodes that current node produces
    
public:
    string getName();
    void setName(string name);
    int getDataWidth();
    void setDataWidth(int dataWidth);
    bool getUsigned();
    void setUsigned(bool usigned);
    vector<node*> getOutgoing();
    void setOutgoing(vector<node*> outgoing);
    vector<node*> getIncoming();
    void setIncoming(vector<node*> incoming);
    void appendOutgoing(node* outgoing);
    void appendIncoming(node* incoming);
    
};
#endif // type_h
