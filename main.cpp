#include "imageAnalysis.h"
#include <QApplication>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QGuiApplication::setApplicationDisplayName(imageAnalysis::tr("Image Analyzer"));
    QCommandLineParser commandLineParser;
    commandLineParser.addHelpOption();
    commandLineParser.addPositionalArgument(imageAnalysis::tr("[file]"), imageAnalysis::tr("Image file to open."));
    commandLineParser.process(QCoreApplication::arguments());
    imageAnalysis imageViewer;
    if (!commandLineParser.positionalArguments().isEmpty()
        && !imageViewer.loadFile(commandLineParser.positionalArguments().front()))
    {
        return -1;
    }
    imageViewer.show();
    return app.exec();
}
