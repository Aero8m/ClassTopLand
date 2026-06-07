#include"./AppLog.h"



void showLog(const char* logText, LogStatus logStatus){
    QString currentTime = QTime::currentTime().toString("hh:mm:ss.zzz");
    if (logStatus == LogStatus::ALL){
        std::cout << RESET << "[" << currentTime.toStdString() << "] " << logText << RESET << std::endl;
    }else if (logStatus == LogStatus::DEBUG){
        std::cout << RESET << "[" << currentTime.toStdString() << "] DEBUG: " << logText << RESET << std::endl;
    }else if (logStatus == LogStatus::INFO){
        std::cout << GREEN << "[" << currentTime.toStdString() << "] INFO: " << logText << RESET << std::endl;
    }else if (logStatus == LogStatus::WARN){
        std::cout << YELLOW << "[" << currentTime.toStdString() << "] WARN: " << logText << RESET << std::endl;
    }else if (logStatus == LogStatus::ERR){
        std::cout << RED << "[" << currentTime.toStdString() << "] ERR: " << logText << RESET << std::endl;
    }else if (logStatus == LogStatus::EXIT){
        std::cout << YELLOW << "[" << currentTime.toStdString() << "] EXIT: " << logText << RESET << std::endl;
    }

}
void showLog(const QString logText, LogStatus logStatus){
    QString currentTime = QTime::currentTime().toString("hh:mm:ss.zzz");
    if (logStatus == LogStatus::ALL){
        std::cout << RESET << "[" << currentTime.toStdString() << "] " << logText.toStdString().c_str() << RESET << std::endl;
    }else if (logStatus == LogStatus::DEBUG){
        std::cout << RESET << "[" << currentTime.toStdString() << "] DEBUG: " << logText.toStdString().c_str()<< RESET << std::endl;
    }else if (logStatus == LogStatus::INFO){
        std::cout << GREEN << "[" << currentTime.toStdString() << "] INFO: " << logText.toStdString().c_str() << RESET << std::endl;
    }else if (logStatus == LogStatus::WARN){
        std::cout << YELLOW << "[" << currentTime.toStdString() << "] WARN: " << logText.toStdString().c_str() << RESET << std::endl;
    }else if (logStatus == LogStatus::ERR){
        std::cout << RED << "[" << currentTime.toStdString() << "] ERR: " << logText.toStdString().c_str() << RESET << std::endl;
    }else if (logStatus == LogStatus::EXIT){
        std::cout << YELLOW << "[" << currentTime.toStdString() << "] EXIT: " << logText.toStdString().c_str() << RESET << std::endl;
    }

}

