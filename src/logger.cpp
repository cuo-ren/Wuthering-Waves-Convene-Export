#include "Logger.h"

void Logger::init() {
    QDir dir("./log");
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    updateLogFile();
    qInstallMessageHandler(messageHandler);
}

void Logger::updateLogFile() {
    //QString today = QDate::currentDate().toString("yyyy-MM-dd");
    QString today = QDate::currentDate().isValid()
        ? QDate::currentDate().toString("yyyy-MM-dd")
        : QString("1970-01-01"); //QLatin1String("unknown-date");

    if (today == currentDate && logFile.isOpen())
        return;

    if (logFile.isOpen())
        logFile.close();

    currentDate = today;
    QString fileName = QString("./log/%1.log").arg(today);
    logFile.setFileName(fileName);
    logFile.open(QIODevice::Append | QIODevice::Text);

    logStream.setDevice(&logFile);
    logStream.setEncoding(QStringConverter::Utf8);

    rotateLogs();
}

void Logger::rotateLogs() {
    QDir dir("./log");
    QStringList allFiles = dir.entryList(QStringList() << "*.log", QDir::Files, QDir::Name);

    QRegularExpression dateLogPattern(R"(^\d{4}-\d{2}-\d{2}\.log$)");
    QStringList dateLogs;

    for (const QString& file : allFiles) {
        if (dateLogPattern.match(file).hasMatch()) {
            dateLogs << file;
        }
    }

    std::sort(dateLogs.begin(), dateLogs.end());

    if (dateLogs.size() > maxLogFiles) {
        int removeCount = dateLogs.size() - maxLogFiles;
        for (int i = 0; i < removeCount; ++i) {
            dir.remove(dateLogs[i]);
        }
    }
}

void Logger::messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg) {
    updateLogFile();

    QString level;
    switch (type) {
    case QtDebugMsg:    level = "[DEBUG]"; break;
    case QtInfoMsg:     level = "[INFO]"; break;
    case QtWarningMsg:  level = "[WARNING]"; break;
    case QtCriticalMsg: level = "[ERROR]"; break;
    case QtFatalMsg:    level = "[FATAL]"; break;
    }
    /*
    QString contextInfo = QString("(%1:%2, %3)")
        .arg(QString::fromUtf8(context.file))
        .arg(context.line)
        .arg(QString::fromUtf8(context.function));
        */
    QString logLine = QString("%1 %2 %3")
        .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
        .arg(level)
        //.arg(contextInfo)
        .arg(msg);

    std::cerr << logLine.toUtf8().constData() << std::endl;
    logStream << logLine << "\n";
    logStream.flush();

    if (type == QtFatalMsg)
        abort();
}
