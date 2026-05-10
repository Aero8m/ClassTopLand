#include"./AppLog.h"



void showLog(const char* LogText,LogStatus LogSt){
    QString CurrentTime = QTime::currentTime().toString("hh:mm:ss.zzz");
    if (LogSt == LogStatus::ALL){
        std::cout << RESET << "[" << CurrentTime.toStdString() << "] " << LogText << RESET << std::endl;
    }else if (LogSt == LogStatus::DEBUG){
        std::cout << RESET << "[" << CurrentTime.toStdString() << "] DEBUG: " << LogText << RESET << std::endl;
    }else if (LogSt == LogStatus::INFO){
        std::cout << GREEN << "[" << CurrentTime.toStdString() << "] INFO: " << LogText << RESET << std::endl;
    }else if (LogSt == LogStatus::WARN){
        std::cout << YELLOW << "[" << CurrentTime.toStdString() << "] WARN: " << LogText << RESET << std::endl;
    }else if (LogSt == LogStatus::ERR){
        std::cout << RED << "[" << CurrentTime.toStdString() << "] ERR: " << LogText << RESET << std::endl;
    }else if (LogSt == LogStatus::EXIT){
        std::cout << YELLOW << "[" << CurrentTime.toStdString() << "] EXIT: " << LogText << RESET << std::endl;
    }

}
void showLog(const QString LogText,LogStatus LogSt){
    QString CurrentTime = QTime::currentTime().toString("hh:mm:ss.zzz");
    if (LogSt == LogStatus::ALL){
        std::cout << RESET << "[" << CurrentTime.toStdString() << "] " << LogText.toStdString().c_str() << RESET << std::endl;
    }else if (LogSt == LogStatus::DEBUG){
        std::cout << RESET << "[" << CurrentTime.toStdString() << "] DEBUG: " << LogText.toStdString().c_str()<< RESET << std::endl;
    }else if (LogSt == LogStatus::INFO){
        std::cout << GREEN << "[" << CurrentTime.toStdString() << "] INFO: " << LogText.toStdString().c_str() << RESET << std::endl;
    }else if (LogSt == LogStatus::WARN){
        std::cout << YELLOW << "[" << CurrentTime.toStdString() << "] WARN: " << LogText.toStdString().c_str() << RESET << std::endl;
    }else if (LogSt == LogStatus::ERR){
        std::cout << RED << "[" << CurrentTime.toStdString() << "] ERR: " << LogText.toStdString().c_str() << RESET << std::endl;
    }else if (LogSt == LogStatus::EXIT){
        std::cout << YELLOW << "[" << CurrentTime.toStdString() << "] EXIT: " << LogText.toStdString().c_str() << RESET << std::endl;
    }

}

