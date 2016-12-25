#include "AsioPiwik.hpp"

//#include <iostream>
//using namespace std;

int main() {
    AsioPiwik::Logger logger(APP_NAME_STRING, 
                             "user_id", 
                             AWESOMEAPP.COM, 
                             SECRECT_DIR+"/piwik/piwik.php");

    logger.addPageVisit({"Main Page", "Visit"});
    logger.log();
}
