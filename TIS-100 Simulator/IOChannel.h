#pragma once

class IOChannel
{
private:
    INode* m_nodeA;
    INode* m_nodeB;
    int m_value;
    INode* m_sender;
    INode* m_receiver;

public:
    IOChannel(INode* a, INode* b);

    void Write(INode* sender, int value);
    void Read(INode* receiver);
    void CancelRead(INode* receiver);
    void CancelWrite(INode* sender);
};