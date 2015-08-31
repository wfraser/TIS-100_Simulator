#pragma once

class IOChannel
{
private:
    struct Endpoint
    {
        INode* node;
        bool writePending;
        int sentValue;
    };

    Endpoint m_a, m_b;

public:
    IOChannel(INode* a, INode* b);

    void Write(INode* senderNode, int value);
    bool Read(INode* receiverNode, int* pValue);
    void CancelWrite(INode* senderNode);

protected:
    void GetEndpoints(INode* node, Endpoint** ppThatEndpoint, Endpoint** ppOtherEndpoint);
};