//////////////////////////////////////////////////////////////////////////
// Proxy.h: interface for the CProxy class.
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PROXY_H__51B3300C_EF6F_41F5_97F3_839F9985A20A__INCLUDED_)
#define AFX_PROXY_H__51B3300C_EF6F_41F5_97F3_839F9985A20A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CProxy  
{
public:
    int StopProxy();
    int StartProxy(int listenport);
    bool WsaStartupOk;
    CProxy();
    virtual ~CProxy();

};

#endif // !defined(AFX_PROXY_H__51B3300C_EF6F_41F5_97F3_839F9985A20A__INCLUDED_)

