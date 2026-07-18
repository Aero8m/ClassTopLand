#pragma once
#include<QUrl>
#define MAKE_EXAPI_URL(apiname,method) (QUrl(QString("https://ctpserver.aero8m.cn/exapi/%1/%2").arg(apiname, method)))
namespace CloudAPIUrl {
	static QUrl MASTER = QUrl("https://ctpserver.aero8m.cn");
	static QString MASTER_STRING = "https://ctpserver.aero8m.cn";
	static QUrl GET_EXAPI_LIST = QUrl(MASTER_STRING + "/exapi/list");
};