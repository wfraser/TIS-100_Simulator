#pragma once

class IOChannel
{
private:
    INode* m_nodeA;
    INode* m_nodeB;
    int m_value;
    INode* m_sender;

public:
    IOChannel(INode* a, INode* b);

    void Write(INode* sender, int value);
    bool Read(INode* receiver, int* pValue);
    void CancelWrite(INode* sender);
};