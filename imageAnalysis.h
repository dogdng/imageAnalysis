#ifndef IMAGEANALYSIS_H
#define IMAGEANALYSIS_H

#include <QWidget>
#include <QtWidgets>
#include <QMainWindow>
#include <QImage>
#ifndef QT_NO_PRINTER
#include <QPrinter>
#endif

#include <QPainter>
#include <QChartGlobal>
#include <QGridLayout>
#include "imgCapWidget.h"
#include "chartWidget.h"

QT_BEGIN_NAMESPACE
class QAction;

class QLabel;
class QMenu;
class QScrollArea;
class QScrollBar;
QT_END_NAMESPACE


QT_CHARTS_USE_NAMESPACE


class imageAnalysis : public QMainWindow
{
    Q_OBJECT


public:
    imageAnalysis();
    ~imageAnalysis();

    bool loadFile(const QString &);
    QGridLayout *gridLayout;


private slots:
    void open();
    void saveAs();
    void print();
    void copy();
    void paste();
    void zoomIn();
    void zoomOut();
    void normalSize();
    void fitToWindow();
    void about();

private:
    void createActions();
    void createMenus();
    void updateActions();
    bool saveFile(const QString &fileName);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    void capturer();
    QImage image;
    QImage newImage;
    QImage imageROI;
    QPixmap pixmapROI;
    imgCapWidget *imageLabel;
    QLabel *imageCaptureLabel;
    QScrollArea *scrollArea;
    chartWidget *histogram;

    double scaleFactor;

#ifndef QT_NO_PRINTER
    QPrinter printer;
#endif

    QAction *saveAsAct;
    QAction *printAct;
    QAction *copyAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *normalSizeAct;
    QAction *fitToWindowAct;
    QChart *createAreaChart() const;
    DataTable generateRandomData(int listCount, int valueMax, int valueCount) const;
    void setImage();
    void setImage(const QImage image);
    void onCompleteCature(QPixmap captureImage);

    DataTable dataForHistogram(QPixmap captureImage);
};



#endif // IMAGEANALYSIS_H
