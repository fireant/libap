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

#ifndef AsioPiwik_hpp
#define AsioPiwik_hpp

#define BOOST_ALL_NO_LIB 1
#define ASIO_STANDALONE
#define ASIO_NO_EXCEPTIONS

#include <asio.hpp>
#include <asio/ssl.hpp>

// Sends log messages to Piwik.
// Example:
//    AsioPiwik::Logger logger("The App Name", 
//                             "user ID", 
//                             "awesomeapp.com");
//    logger.addPageVisit({"Section B", "Subsection N"});
//    logger.addPageVisit({"Section H"});
//    logger.log();
namespace AsioPiwik {
    class Logger
    {
    public:
        Logger(std::string appName,
               std::string uid,
               std::string serverDomainName,
               std::string endpoint="/piwik.php",
               unsigned port=443,
               unsigned thresholdNumEntries=10,
               unsigned bufferLimit=30);

        // Adds a log message for visiting a path to the buffered list.
        // The path is created from the list of strings, e.g.,
        // {"Sec A", "Sub B"} registers this path in Piwik: "Sec A/Sub B".
        // A list of string pairs can be given to be passed to Piwik, e.g.,
        // {{"h","3"}, {"m", "0"}, {"s", "10"}} can be used to set the local time.
        // If the buffered list is longer than the set threshold the log() method 
        // is called.
        void addPageVisit(std::initializer_list<std::string> list,
                          std::initializer_list<std::pair<std::string,
                                                          std::string>> opts={});

        // Sets the function that should receive the flag whether the attempt
        // to send the messages to Piwik was successful.
        void setCallback(std::function<void(bool)> callback) {
            this->callback = callback;
        }
        
        // Attempts to send the buffered messages to Piwik.
        // To know whether the attempt was successfull use the setCallBack(..)
        // method.
        void log();
        
    private:
        bool verify_certificate(bool preverified,
                                asio::ssl::verify_context& ctx);
        void handle_connect(const asio::error_code& error);
        void handle_handshake(const asio::error_code& error);
        std::string bakeRequest();
        void send(const std::string& request);
        void handle_write(const asio::error_code& error, size_t bytes_transferred);
        void handle_read(const asio::error_code& error,  size_t bytes_transferred);
        
        void callCallback(bool sent);

        std::unique_ptr<asio::ssl::stream<asio::ip::tcp::socket>> socket;
        std::unique_ptr<asio::io_service> service;
        std::unique_ptr<asio::ip::tcp::resolver> resolver;
        std::unique_ptr<asio::ip::tcp::resolver::query> query;
        std::unique_ptr<asio::ip::tcp::resolver::iterator> iterator;
        std::unique_ptr<asio::ssl::context> ctx;

        std::string appName;
        std::string uid;
        std::string domainName;
        std::string endpoint;
        std::vector<std::string> messageList;
        std::vector<std::string> sendList;
        std::string serverResponse;
        unsigned thresholdNumEntries;
        unsigned bufferLimit;
        std::function<void(bool)> callback;
    };
}

#endif /* AsioPiwik_hpp */
