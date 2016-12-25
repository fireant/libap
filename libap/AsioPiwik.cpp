// Copyright (c) 2016 Mosalam Ebrahimi
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "AsioPiwik.hpp"
#include <iostream>

using namespace AsioPiwik;
using namespace asio;
using namespace asio::ip;
using namespace std::placeholders;

using namespace std;

template <typename Exception>
void asio::detail::throw_exception(const Exception& e)
{
    cerr<<"ASIO throwed an exception"<<endl;
}

Logger::Logger(string appName,
               string uid,
               string serverDomainName,
               string endpoint,
               unsigned port,
               unsigned thresholdNumEntries,
               unsigned bufferLimit):
    uid{uid}, domainName(serverDomainName),
    endpoint(endpoint), thresholdNumEntries(thresholdNumEntries),
    bufferLimit(bufferLimit), callback(nullptr) {
    
    service = unique_ptr<io_service>(new io_service);
    
    resolver = unique_ptr<tcp::resolver>(new tcp::resolver(*service));
    query =  unique_ptr<tcp::resolver::query>(new tcp::resolver::query(serverDomainName,
                                                                       to_string(port)));
    iterator = unique_ptr<tcp::resolver::iterator>(new tcp::resolver::iterator(resolver->resolve(*query)));

    ctx = unique_ptr<ssl::context>(new ssl::context(ssl::context::sslv23));
    ctx->set_default_verify_paths();

    socket = unique_ptr<ssl::stream<tcp::socket>> (new ssl::stream<tcp::socket>(*service, *ctx));
    socket->set_verify_mode(ssl::verify_peer);
    socket->set_verify_callback(bind(&Logger::verify_certificate, this, _1, _2));
}

void Logger::log() {
    if (messageList.size() == 0) {
        return;
    }

    if (sendList.size() > bufferLimit) {
        sendList.clear();
    }

    for (string& entry : messageList) {
        sendList.push_back(entry);
    }
    messageList.clear();


    asio::async_connect(socket->lowest_layer(), *iterator,
                        bind(&Logger::handle_connect, this, _1));
    service->run();
}

void Logger::addPageVisit(initializer_list<string> list,
                          initializer_list<pair<std::string, string>> opts) {
    if (list.size() < 1) {
        return;
    }

    string url;
    string title;
    for (auto& page : list) {
        url += page + "/";
        title = page;
    }

    string message = "\"?idsite=1&url=http://" + domainName;
    message += "/" + url;
    message += "&action_name=" + title;
    message += "&uid=" + uid;

    for (auto& opt : opts) {
        message += "&" + opt.first + "=" + opt.second;
    }

    message += "&rec=1\"";

    messageList.push_back(message);
    
    if (messageList.size() > thresholdNumEntries) {
        log();
    }
}

bool Logger::verify_certificate(bool preverified,
                                ssl::verify_context& ctx){
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    // here can check the subject_name is correct
    
    return true;
}

void Logger::handle_connect(const error_code& error){
    if (!error){
        socket->async_handshake(ssl::stream_base::client,
                                bind(&Logger::handle_handshake, this, _1));
    }
    else {
        callCallback(false);
    }
}

string Logger::bakeRequest() {
    string post_body = "{\"requests\":[";
    post_body += sendList[0];
    for (int i=1; i<sendList.size(); ++i) {
        post_body += ",\"";
        post_body += sendList[i]+"\"";
    }
    post_body +="]}";
    
    string request;
    request += "POST " + endpoint + " HTTP/1.1 \r\n";
    request += "Host:" + domainName + "\r\n";
    request += "User-Agent: C/1.0";
    request += "Content-Type: application/json; charset=utf-8 \r\n";
    request += "Accept: */*\r\n";
    request += "Content-Length: " + to_string(post_body.length()) + "\r\n";
    request += "Connection: close\r\n\r\n";
    request += post_body;
    
    return request;
}

void Logger::handle_handshake(const error_code& error){
    if (!error) {
        send(bakeRequest().c_str());
    }
    else{
        callCallback(false);
    }
}

void Logger::send(const string& request){
    asio::async_write(*socket,
                      asio::buffer(request),
                      bind(&Logger::handle_write, this, _1, _2));
}

void Logger::handle_write(const error_code& error, size_t bytes_transferred){
    if (!error) {
        serverResponse.resize(bytes_transferred);
        asio::async_read(*socket,
                         asio::buffer(&serverResponse[0], bytes_transferred),
                         bind(&Logger::handle_read, this, _1, _2));
    }
    else {
        callCallback(false);
    }
}

void Logger::handle_read(const error_code& error, size_t bytes_transferred) {
    if (!error)  {
        sendList.clear();
        callCallback(true);
    }
    else {
        callCallback(false);
    }
}

void Logger::callCallback(bool sent) {
    if (callback == nullptr) {
        return;
    }

    callback(sent);
}

