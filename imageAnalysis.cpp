#include "imageAnalysis.h"
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
    #if QT_CONFIG(printdialog)
    #include <QPrintDialog>
    #endif
#endif
#include <QtCharts/QChartView>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>

imageAnalysis::imageAnalysis()
    : imageLabel(new imgCapWidget)
    , scrollArea(new QScrollArea)
    , scaleFactor(1)
{
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *horizontalLayout = new QHBoxLayout(centralWidget);
    horizontalLayout->setSpacing(6);
    horizontalLayout->setContentsMargins(11, 11, 11, 11);

    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    scrollArea->setVisible(true);

    QHBoxLayout *horizontalLayout2 = new QHBoxLayout();
    horizontalLayout2->setSpacing(4);
    horizontalLayout2->setSizeConstraint(QLayout::SetMinAndMaxSize);
    horizontalLayout2->addWidget(scrollArea);

    QVBoxLayout *verticalLayout = new QVBoxLayout();
    verticalLayout->setSpacing(6);

    imageCaptureLabel = new QLabel();

    QScrollArea *scrollAreaCap = new QScrollArea();
    scrollAreaCap->setBackgroundRole(QPalette::Dark);
    scrollAreaCap->setWidget(imageCaptureLabel);
    scrollAreaCap->setVisible(true);

    verticalLayout->addWidget(scrollAreaCap);

    //右下
    QTabWidget *tabWidget = new QTabWidget(centralWidget);
    QWidget *tab3 = new QWidget();

    tabWidget->addTab(tab3, QString("Histogram 直方图"));
    histogram = new chartWidget();
    QHBoxLayout *layout=new QHBoxLayout;//使直方图充满tab
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(histogram);
    tab3->setLayout(layout);

    QWidget *tab4 = new QWidget();
    tabWidget->addTab(tab4, QString());

    verticalLayout->addWidget(tabWidget);
    verticalLayout->setStretch(0,1);
    verticalLayout->setStretch(1,1);

    horizontalLayout2->addLayout(verticalLayout);

    horizontalLayout2->setStretch(0, 1);
    horizontalLayout2->setStretch(1, 1);

    horizontalLayout->addLayout(horizontalLayout2);

    this->setCentralWidget(centralWidget);

    tabWidget->setCurrentIndex(0);

    createActions();
    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

    connect(imageLabel, &imgCapWidget::signalCompleteCature, this, &imageAnalysis::onCompleteCature);

}

imageAnalysis::~imageAnalysis()
{

}
bool imageAnalysis::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    newImage = reader.read();
    if (newImage.isNull())
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    setImage();

    setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(image.width()).arg(image.height()).arg(image.depth());
    statusBar()->showMessage(message);

    return true;
}

void imageAnalysis::onCompleteCature(QPixmap captureImage)
{
    this->imageCaptureLabel->setPixmap(captureImage);//显示截取的图片
    this->imageCaptureLabel->setGeometry(0, 0, captureImage.width(),captureImage.height());//可以加个滚动条

    histogram->getData(dataForHistogram(captureImage));
}

void imageAnalysis::setImage()
{
    image = newImage;
    imageLabel->setPixmap(QPixmap::fromImage(image));
    imageLabel->loadPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();
}

void imageAnalysis::setImage(const QImage image)
{
    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    scrollArea->setVisible(true);
    printAct->setEnabled(true);
    fitToWindowAct->setEnabled(true);
    updateActions();

    if (!fitToWindowAct->isChecked())
        imageLabel->adjustSize();
}

bool imageAnalysis::saveFile(const QString &fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(image))
    {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog)
    {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }

    QStringList mimeTypeFilters;
    const QByteArrayList supportedMimeTypes = acceptMode == QFileDialog::AcceptOpen
        ? QImageReader::supportedMimeTypes() : QImageWriter::supportedMimeTypes();
    foreach (const QByteArray &mimeTypeName, supportedMimeTypes)
        mimeTypeFilters.append(mimeTypeName);
    mimeTypeFilters.sort();
    dialog.setMimeTypeFilters(mimeTypeFilters);
    dialog.selectMimeTypeFilter("image/jpeg");
    if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}

void imageAnalysis::open()
{
    QFileDialog dialog(this, tr("Open File"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptOpen);

    while (dialog.exec() == QDialog::Accepted && !loadFile(dialog.selectedFiles().first()))
    {}
}

void imageAnalysis::saveAs()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first()))
    {}
}

void imageAnalysis::print()
{
    Q_ASSERT(imageLabel->pixmap());
#if QT_CONFIG(printdialog)
    QPrintDialog dialog(&printer, this);
    if (dialog.exec())
    {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageLabel->pixmap());
    }
#endif
}

void imageAnalysis::copy()
{
#ifndef QT_NO_CLIPBOARD
    QGuiApplication::clipboard()->setImage(image);
#endif // !QT_NO_CLIPBOARD
}

#ifndef QT_NO_CLIPBOARD
static QImage clipboardImage()
{
    if (const QMimeData *mimeData = QGuiApplication::clipboard()->mimeData())
    {
        if (mimeData->hasImage())
        {
            const QImage image = qvariant_cast<QImage>(mimeData->imageData());
            if (!image.isNull())
                return image;
        }
    }
    return QImage();
}
#endif // !QT_NO_CLIPBOARD

void imageAnalysis::paste()
{
#ifndef QT_NO_CLIPBOARD
    const QImage clipImage = clipboardImage();
    if (clipImage.isNull())
    {
        statusBar()->showMessage(tr("No image in clipboard"));
    }
    else
    {
        setImage(clipImage);
        setWindowFilePath(QString());
        const QString message = tr("Obtained image from clipboard, %1x%2, Depth: %3")
            .arg(clipImage.width()).arg(clipImage.height()).arg(clipImage.depth());
        statusBar()->showMessage(message);
    }
#endif // !QT_NO_CLIPBOARD
}

void imageAnalysis::zoomIn()
{
    scaleImage(1.25);
}

void imageAnalysis::zoomOut()
{
    scaleImage(0.8);
}

void imageAnalysis::normalSize()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
}

void imageAnalysis::fitToWindow()
{
    bool fitToWindow = fitToWindowAct->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        normalSize();
    updateActions();
}

void imageAnalysis::about()
{
    QMessageBox::about(this, tr("About Image Analyzer"),
            tr("<p>The <b>Image Analyzer</b> is a tool to analysis the image.</p>"
               "<p>It is easy to use and .....</p>"
               "<p>    </p>"
               "<p>    </p>"
               "<p>Author : dogdng </p>"
               "<p> Have fun.</p>"));
}

void imageAnalysis::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));

    QAction *openAct = fileMenu->addAction(tr("&Open..."), this, &imageAnalysis::open);
    openAct->setShortcut(QKeySequence::Open);

    saveAsAct = fileMenu->addAction(tr("&Save As..."), this, &imageAnalysis::saveAs);
    saveAsAct->setEnabled(false);

    printAct = fileMenu->addAction(tr("&Print..."), this, &imageAnalysis::print);
    printAct->setShortcut(QKeySequence::Print);
    printAct->setEnabled(false);

    fileMenu->addSeparator();

    QAction *exitAct = fileMenu->addAction(tr("E&xit"), this, &QWidget::close);
    exitAct->setShortcut(tr("Ctrl+Q"));

    QMenu *editMenu = menuBar()->addMenu(tr("&Edit"));

    copyAct = editMenu->addAction(tr("&Copy"), this, &imageAnalysis::copy);
    copyAct->setShortcut(QKeySequence::Copy);
    copyAct->setEnabled(false);

    QAction *pasteAct = editMenu->addAction(tr("&Paste"), this, &imageAnalysis::paste);
    pasteAct->setShortcut(QKeySequence::Paste);

    QMenu *viewMenu = menuBar()->addMenu(tr("&View"));

    zoomInAct = viewMenu->addAction(tr("Zoom &In (25%)"), this, &imageAnalysis::zoomIn);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    zoomInAct->setEnabled(false);

    zoomOutAct = viewMenu->addAction(tr("Zoom &Out (25%)"), this, &imageAnalysis::zoomOut);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    zoomOutAct->setEnabled(false);

    normalSizeAct = viewMenu->addAction(tr("&Normal Size"), this, &imageAnalysis::normalSize);
    normalSizeAct->setShortcut(tr("Ctrl+S"));
    normalSizeAct->setEnabled(false);

    viewMenu->addSeparator();

    fitToWindowAct = viewMenu->addAction(tr("&Fit to Window"), this, &imageAnalysis::fitToWindow);
    fitToWindowAct->setEnabled(false);
    fitToWindowAct->setCheckable(true);
    fitToWindowAct->setShortcut(tr("Ctrl+F"));

    QMenu *helpMenu = menuBar()->addMenu(tr("&Help"));

    helpMenu->addAction(tr("&About"), this, &imageAnalysis::about);
    //helpMenu->addAction(tr("About &Qt"), &QApplication::aboutQt);
}

void imageAnalysis::updateActions()
{
    saveAsAct->setEnabled(!image.isNull());
    copyAct->setEnabled(!image.isNull());
    zoomInAct->setEnabled(!fitToWindowAct->isChecked());
    zoomOutAct->setEnabled(!fitToWindowAct->isChecked());
    normalSizeAct->setEnabled(!fitToWindowAct->isChecked());
}

void imageAnalysis::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    zoomInAct->setEnabled(scaleFactor < 3.0);
    zoomOutAct->setEnabled(scaleFactor > 0.333);
}

void imageAnalysis::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

DataTable imageAnalysis::dataForHistogram(QPixmap captureImage)
{
    DataTable dataTable;
    QImage inputImage = captureImage.toImage();
    QColor pixelColor;
    int count[3][256]={{0}};
    memset(count,0,sizeof(count));

    for(int i(0); i<inputImage.height(); i++)
    {
        for(int j(0); j<inputImage.width(); j++)
        {
            pixelColor = QColor(inputImage.pixel(j,i));
            count[0][pixelColor.red()]++;
            count[1][pixelColor.green()]++;
            count[2][pixelColor.blue()]++;
        }
    }
    dataTable.clear();
    dataTable.emplace_back(0);
    dataTable.emplace_back(0);
    dataTable.emplace_back(0);
    for(int i(0); i<256; i++)
    {
        dataTable[0].emplace_back(count[0][i]);
        dataTable[1].emplace_back(count[1][i]);
        dataTable[2].emplace_back(count[2][i]);
    }

    return dataTable;
}
