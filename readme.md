# libap

An Asio-based cross-platform library in C++ for Piwik. 


[Piwik](https://piwik.org) is a versatile analytics platform, which can also be used for mobile app analytics. *libap* is an easy to use class, using [Asio](http://think-async.com) to send log messages to your Piwik server. The non-Boost Asio, a header only library, is used to make it easy to add the code to your apps.

To learn how to use it, install Piwik on your server and then see the comments in libap/AsioPiwik.hpp and test/main.cpp. If you want to make sure the messages are delivered even after the main process is terminated, you should use some OS-specific code. For instance, on iOS you could call `beginBackgroundTaskWithExpirationHandler`, before calling `addPageVisit` and `log`.
