#ifndef IMGCAPWIDGET_H
#define IMGCAPWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPainter>

enum CaptureState{
    InitCapture = 0,
    BeginCaptureImage,
    FinishCaptureImage,
    BeginMoveCaptureArea,
    FinishMoveCaptureArea,
    BeginMoveStretchRect,
    FinishMoveStretchRect
}; //进行截屏的状态;
enum StretchRectState{
    NotSelect = 0,
    TopLeftRect,
    TopRightRect,
    BottomLeftRect,
    BottomRightRect,
    LeftCenterRect,
    TopCenterRect,
    RightCenterRect,
    BottomCenterRect
};// 当前鼠标所在顶点状态;

class imgCapWidget : public QLabel
{
    Q_OBJECT
public:
    imgCapWidget();
    ~imgCapWidget();
    void loadPixmap(QPixmap pixmap);
Q_SIGNALS:
    // 通知截图完毕，并将截取图片传递给调用类;
    void signalCompleteCature(QPixmap catureImage);

private:
    QImage newImage;

    void initStretchRect();

    QRect getRect(const QPoint &beginPoint, const QPoint &endPoint);
    QRect getMoveRect();
    QRect getStretchRect();
    bool isPressPointInSelectRect(QPoint mousePressPoint);
    QRect getSelectRect();
    QPoint getMovePoint();
    StretchRectState getStrethRectState(QPoint point);
    void setStretchCursorStyle(StretchRectState stretchRectState);

    void drawCaptureImage();
    void drawStretchRect();

    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *event);

    void drawSelectRectInfo();

    QPixmap loadedPixmap, capturePixmap;
    int imageWidth;
    int imageHeight;
    // 保存确定选区的坐标点;
    QPoint beginPoints, endPoints, beginMovePoint, endMovePoint;
    QPainter painter;
    // 保存当前截图状态;
    CaptureState currentCaptureState;
    // 当前选择区域矩形;
    QRect currentSelectRect;
    // 选中矩形8个顶点小矩形;
    QRect topLeftRect, topRightRect, bottomLeftRect, bottomRightRect;
    QRect leftCenterRect, topCenterRect, rightCenterRect, bottomCenterRect;
    // 当前鼠标所在顶点状态;
    StretchRectState stretchRectState;
    void restartCapture();
};

#endif // IMGCAPWIDGET_H
