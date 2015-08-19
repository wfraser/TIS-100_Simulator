#pragma once

class IOChannel
{
private:
    struct Endpoint
    {
        INode* node;
        bool readPending;
        bool writePending;
        int sentValue;
    };

    Endpoint m_a, m_b;

public:
    IOChannel(INode* a, INode* b);

    void Write(INode* senderNode, int value);
    void Read(INode* receiverNode);
    void CancelRead(INode* receiverNode);
    void CancelWrite(INode* senderNode);

protected:
    void GetEndpoints(INode* node, Endpoint** ppThatEndpoint, Endpoint** ppOtherEndpoint);
};