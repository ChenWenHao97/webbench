#pragma once
#include <iostream>
#include <vector>
#include <mutex>
#include <string>
#include <getopt.h>
#include <stdio.h>
#include <unistd.h>
#include <regex>
#include <utility>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "../core/Parser.hpp"
using namespace std;
class HttpBuild
{
public:
    int port;
    bool isLegalURL(string url)
    {
        if (url.size() > 1500)
        {
            cout << "url长度过长" << endl;
            return false;
        }
        string pattern{"(https?)://.*(.cn|.com|.htm|.html|.aspx?|.jsp|.php|.net||#)$"};
        regex re(pattern);

        return regex_match(url, re);
    }
    int GetPort(string url) //解析url里面端口号，默认http为80
    {

        if (url[4] == 's')
            port = 443;
        if (isIP(url))
        {
            string pattern{"(.*).(.*).(.*).(.*):(.*)"};
            smatch res;
            regex re(pattern);
            if (regex_search(url, res, re))
            {

                return stoi(res[5]);
            }
            return port;
        }
        if (!isLegalURL(url))
            return -1;
        string pattern{"(.*):(.*):([0-9]{0,5})"};
        smatch res;
        regex re(pattern);
        if (regex_search(url, res, re))
            return stoi(res[3]);
        return port;
    }
    string GetHostByURL(string url)
    {
        if (url[4] == 's')
            port = 443;
        if (!isLegalURL(url))
            return "";
        string pattern1{"(.*)://(.*)/"};
        string pattern2{"(.*)://(.*)"};
        smatch res;
        regex re1(pattern1);
        regex re2(pattern2);
        if (regex_search(url, res, re1))
        {
            return res[2];
        }
        else if (regex_search(url, res, re2))
        {
            return res[2];
        }
        cout << "GetHostByURL  no value" << endl;
        return "";
    }
    string GetIpByURL(string url) //使用gethostbyname函数得到hostname
    {
        if (isIP(url))
            return url;
        const char *name = GetHostByURL(url).c_str();

        struct hostent *hptr;
        hptr = gethostbyname(name);
        if (hptr == NULL)
        {
            cout << "gethostbyname error for host:" << name << ":" << hstrerror(h_errno) << endl;
            return "";
        }
        //输出主机的别名
        char **pptr;
        char str[INET_ADDRSTRLEN];

        //输出ip地址
        switch (hptr->h_addrtype)
        {
        case AF_INET:
            pptr = hptr->h_addr_list;
            // for (; *pptr!=NULL; pptr++)
            // {
            //     printf("\taddress: %s\n",
            //             inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str)));
            // }
            return inet_ntop(hptr->h_addrtype, hptr->h_addr, str, sizeof(str));
            break;
        default:
            cout << "unknown address type" << endl;
            break;
        }

        return "";
    }
    string GetCatalogue(string url)
    {
        if (isIP(url))
            return "/";
        if (!isLegalURL(url))
            return "";

        string pattern{"(https?)://.*(.cn|.com|.htm|.html|.aspx?|.jsp|.php|.net)/(.*)"};
        smatch res;
        regex re(pattern);

        if (regex_search(url, res, re))
        {
            string str(res[3]);
            return "/" + str;
        }
        return "/"; //默认访问根
    }
    string BuildHttpRequest(string url)
    {
        string HttpRquest = "";
        string catalogue = GetCatalogue(url);

        string Accept = "Accept:text/xml;Content-Type:text/html\r\n";
        string Referer = "Referer:" + url + "\r\n";
        string AcceptLanguage = "Accept-Language: zh-cn\r\n";
        string AcceptCharst = "Accept-Charset: GB2312,utf-8;q=0.7,*;q=0.7\r\n";
        string AcceptEncoding = "Accept-Encoding: gzip, deflate\r\n";
        string UserAgent = "Agent: Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.1; SV1; .NET CLR 2.0.50727; TheWorld)\r\n";

        string Host;
        if (isIP(url))
            Host = url;
        else
            Host = GetHostByURL(url);
        string Longcon = "Connection: Keep-Alive\r\n";

        bool http10 = Parser::get_instance().http10;

        bool http11 = Parser::get_instance().http11;

        switch (Parser::get_instance().method)
        {
        case METHOD_GET:
            HttpRquest = HttpRquest + "GET ";
            break;
        case METHOD_HEAD:
            HttpRquest = HttpRquest + "HEAD ";
            break;
        case METHOD_OPTIONS:
            HttpRquest = HttpRquest + "OPTIONS ";
            break;
        case METHOD_TRACE:
            HttpRquest = HttpRquest + "TRACE ";
            break;
        }
        if (http10)
        {
            HttpRquest += GetCatalogue(url);
            HttpRquest += " HTTP/1.0\r\n";
            HttpRquest += Referer;
            HttpRquest += "Host:" + Host + "\r\n";

            HttpRquest += UserAgent;
            HttpRquest += Accept;
            HttpRquest += AcceptLanguage;
            HttpRquest += AcceptEncoding;
            HttpRquest += AcceptCharst + "\r\n";
        }
        else if (http11)
        {
            HttpRquest += GetCatalogue(url);
            HttpRquest += " HTTP/1.1\r\n";
            HttpRquest += Referer;
            HttpRquest += "Host:" + Host;

            HttpRquest += UserAgent;
            HttpRquest += Accept;
            HttpRquest += AcceptLanguage;
            HttpRquest += AcceptEncoding;
            HttpRquest += AcceptCharst;
            HttpRquest += Longcon;
            HttpRquest += "\r\n";
        }

        return HttpRquest;
    }
    bool isIP(string url)
    {
        string pattern{"(\\d+).(\\d+).(\\d+).(\\d+)"}; //直接输入ip的情况
        smatch res;
        regex re(pattern);
        if (regex_match(url, res, re))
        {
            return true;
        }
        return false;
    }
    HttpBuild() : port(80) {}
};
