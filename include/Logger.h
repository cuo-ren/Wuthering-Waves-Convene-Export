#pragma once
#include <QTextStream>
#include <QDebug>
#include <QDate>
#include <QDir>
#include <QFile>
#include <QRegularExpression>
#include <iostream>

class Logger
{
public:
	static void init();
private:
    static void updateLogFile();
    static void rotateLogs();
    static void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);

    static inline QFile logFile;
    static inline QTextStream logStream;
    static inline QString currentDate;
    static constexpr int maxLogFiles = 7;
};