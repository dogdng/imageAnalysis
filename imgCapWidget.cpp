#include "imgCapWidget.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMouseEvent>


// 矩形选中区边框宽度;
#define SELECT_RECT_BORDER_WIDTH                1

// 选中矩形8个拖拽点小矩形的宽高;
#define STRETCH_RECT_WIDTH                      6
#define STRETCH_RECT_HEIGHT                     6

// 选中矩形信息矩形的宽高;
#define SELECT_RECT_INFO_WIDTH                  75
#define SELECT_RECT_INFO_HEIGHT                 20

// 选区矩形的右下顶点放大图的宽高;
#define END_POINT_RECT_WIDTH                    122
#define END_POINT_RECT_HEIGHT                   122
#define END_POINT_IMAGE_HEIGHT                  90


imgCapWidget::imgCapWidget() : QLabel(), currentCaptureState(InitCapture)
{
    setFocusPolicy(Qt::StrongFocus);
    //this->setMouseTracking(true);
}

imgCapWidget::~imgCapWidget()
{

}

void imgCapWidget::initStretchRect()
{
    stretchRectState = NotSelect;
    topLeftRect = QRect(0 , 0 , 0 , 0);
    topRightRect = QRect(0, 0, 0, 0);
    bottomLeftRect = QRect(0, 0, 0, 0);
    bottomRightRect = QRect(0, 0, 0, 0);

    leftCenterRect = QRect(0, 0, 0, 0);
    topCenterRect = QRect(0, 0, 0, 0);
    rightCenterRect = QRect(0, 0, 0, 0);
    bottomCenterRect = QRect(0, 0, 0, 0);
}

// 获取图片;
void imgCapWidget::loadPixmap(QPixmap pixmap)
{
    loadedPixmap = pixmap;//QPixmap::grabWindow(QApplication::desktop()->winId()); //抓取当前屏幕的图片;
    //loadPixmap =
    imageWidth = loadedPixmap.width();
    imageHeight = loadedPixmap.height();
    //setGeometry(0,0,imageWidth,imageHeight);
    initStretchRect();
}

void imgCapWidget::mousePressEvent(QMouseEvent *event)
{
    stretchRectState = getStrethRectState(event->pos());
    if (event->button() == Qt::LeftButton)
    {
        if (currentCaptureState == InitCapture)
        {
            currentCaptureState = BeginCaptureImage;
            beginPoints = event->pos();
        }
        // 是否在拉伸的小矩形中;
        else if (stretchRectState != NotSelect)
        {
            currentCaptureState = BeginMoveStretchRect;
            // 当前鼠标在拖动选中区顶点时,设置鼠标当前状态;
            setStretchCursorStyle(stretchRectState);
            beginMovePoint = event->pos();
        }
        // 是否在选中的矩形中;
        else if (isPressPointInSelectRect(event->pos()))
        {
            currentCaptureState = BeginMoveCaptureArea;
            beginMovePoint = event->pos();
        }
    }

    return QWidget::mousePressEvent(event);
}

void imgCapWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (currentCaptureState == BeginCaptureImage)
    {
        endPoints = event->pos();
        update();
    }
    else if (currentCaptureState == BeginMoveCaptureArea)
    {
        endMovePoint = event->pos();
        update();
    }
    else if (currentCaptureState == BeginMoveStretchRect)
    {
        endMovePoint = event->pos();
        update();
        // 当前鼠标在拖动选中区顶点时,在鼠标未停止移动前，一直保持鼠标当前状态;
        return QWidget::mouseMoveEvent(event);
    }


    // 根据鼠标是否在选中区域内设置鼠标样式;
    StretchRectState stretchRectState = getStrethRectState(event->pos());
    if (stretchRectState != NotSelect)
    {
        setStretchCursorStyle(stretchRectState);
    }
    else if (isPressPointInSelectRect(event->pos()))
    {
        setCursor(Qt::SizeAllCursor);
    }
    else if (!isPressPointInSelectRect(event->pos()) && currentCaptureState != BeginMoveCaptureArea)
    {
        setCursor(Qt::ArrowCursor);
    }

    return QWidget::mouseMoveEvent(event);
}

void imgCapWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (currentCaptureState == BeginCaptureImage)
    {
        currentCaptureState = FinishCaptureImage;
        endPoints = event->pos();
        update();
    }
    else if (currentCaptureState == BeginMoveCaptureArea)
    {
        currentCaptureState = FinishMoveCaptureArea;
        endMovePoint = event->pos();
        update();
    }
    else if (currentCaptureState == BeginMoveStretchRect)
    {
        currentCaptureState = FinishMoveStretchRect;
        endMovePoint = event->pos();
        update();
    }

    return QWidget::mouseReleaseEvent(event);
}

void imgCapWidget::keyPressEvent(QKeyEvent *event)
{
    // Esc 键退出截图;
    if (event->key() == Qt::Key_Escape)
    {
        restartCapture();//close();
    }
    // Eeter键完成截图;
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        signalCompleteCature(capturePixmap);
        restartCapture();//close();
    }
}

// 当前鼠标坐标是否在选取的矩形区域内;
bool imgCapWidget::isPressPointInSelectRect(QPoint mousePressPoint)
{
    QRect selectRect = getRect(beginPoints, endPoints);
    if (selectRect.contains(mousePressPoint))
    {
        return true;
    }

    return false;
}

void imgCapWidget::paintEvent(QPaintEvent *event)
{
    painter.begin(this);          //进行重绘;

    QColor shadowColor = QColor(0, 0, 0, 80);                      //阴影颜色设置;
    painter.setPen(QPen(QColor(0 , 174 , 255), 1, Qt::SolidLine, Qt::FlatCap));    //设置画笔;
    painter.drawPixmap(0, 0, loadedPixmap);                       //将背景图片画到窗体上;
    painter.fillRect(loadedPixmap.rect(), shadowColor);           //画影罩效果;
    drawSelectRectInfo();
    switch (currentCaptureState)
    {
    case InitCapture:
        break;
    case BeginCaptureImage:
    case FinishCaptureImage:
    case BeginMoveCaptureArea:
    case FinishMoveCaptureArea:
    case BeginMoveStretchRect:
    case FinishMoveStretchRect:
        currentSelectRect = getSelectRect();
        drawCaptureImage();
    default:
        break;
    }

    painter.end();  //重绘结束;
}

// 根据当前截取状态获取当前选中的截图区域;
QRect imgCapWidget::getSelectRect()
{
    if (currentCaptureState == BeginCaptureImage || currentCaptureState == FinishCaptureImage)
    {
        return getRect(beginPoints, endPoints);
    }
    else if (currentCaptureState == BeginMoveCaptureArea || currentCaptureState == FinishMoveCaptureArea)
    {
        return getMoveRect();
    }
    else if (currentCaptureState == BeginMoveStretchRect || currentCaptureState == FinishMoveStretchRect)
    {
        return getStretchRect();
    }

    return QRect(0, 0, 0, 0);
}

// 绘制当前选中的截图区域;
void imgCapWidget::drawCaptureImage()
{
    capturePixmap = loadedPixmap.copy(currentSelectRect);
    painter.drawPixmap(currentSelectRect.topLeft(), capturePixmap);
    painter.drawRect(currentSelectRect);
    drawStretchRect();
}

// 绘制选中矩形各拖拽点小矩形;
void imgCapWidget::drawStretchRect()
{
    QColor color = QColor(0, 174, 255);
    // 四个角坐标;
    QPoint topLeft = currentSelectRect.topLeft();
    QPoint topRight = currentSelectRect.topRight();
    QPoint bottomLeft = currentSelectRect.bottomLeft();
    QPoint bottomRight = currentSelectRect.bottomRight();
    // 四条边中间点坐标;
    QPoint leftCenter = QPoint(topLeft.x(), (topLeft.y() + bottomLeft.y()) / 2);
    QPoint topCenter = QPoint((topLeft.x() + topRight.x()) / 2, topLeft.y());
    QPoint rightCenter = QPoint(topRight.x(), leftCenter.y());
    QPoint bottomCenter = QPoint(topCenter.x(), bottomLeft.y());

    topLeftRect = QRect(topLeft.x() - STRETCH_RECT_WIDTH / 2, topLeft.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    topRightRect = QRect(topRight.x() - STRETCH_RECT_WIDTH / 2, topRight.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    bottomLeftRect = QRect(bottomLeft.x() - STRETCH_RECT_WIDTH / 2, bottomLeft.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    bottomRightRect = QRect(bottomRight.x() - STRETCH_RECT_WIDTH / 2, bottomRight.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);

    leftCenterRect = QRect(leftCenter.x() - STRETCH_RECT_WIDTH / 2, leftCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    topCenterRect = QRect(topCenter.x() - STRETCH_RECT_WIDTH / 2, topCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    rightCenterRect = QRect(rightCenter.x() - STRETCH_RECT_WIDTH / 2, rightCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);
    bottomCenterRect = QRect(bottomCenter.x() - STRETCH_RECT_WIDTH / 2, bottomCenter.y() - STRETCH_RECT_HEIGHT / 2, STRETCH_RECT_WIDTH, STRETCH_RECT_HEIGHT);

    painter.fillRect(topLeftRect, color);
    painter.fillRect(topRightRect, color);
    painter.fillRect(bottomLeftRect, color);
    painter.fillRect(bottomRightRect, color);
    painter.fillRect(leftCenterRect, color);
    painter.fillRect(topCenterRect, color);
    painter.fillRect(rightCenterRect, color);
    painter.fillRect(bottomCenterRect, color);
}

// 根据beginPoint , endPoint 获取当前选中的矩形;
QRect imgCapWidget::getRect(const QPoint &beginPoint, const QPoint &endPoint)
{
    int x, y, width, height;
    width = qAbs(beginPoint.x() - endPoint.x());
    height = qAbs(beginPoint.y() - endPoint.y());
    x = beginPoint.x() < endPoint.x() ? beginPoint.x() : endPoint.x();
    y = beginPoint.y() < endPoint.y() ? beginPoint.y() : endPoint.y();

    QRect selectedRect = QRect(x, y, width, height);
    // 避免宽或高为零时拷贝截图有误;
    // 可以看QQ截图，当选取截图宽或高为零时默认为2;
    if (selectedRect.width() == 0)
    {
        selectedRect.setWidth(1);
    }
    if (selectedRect.height() == 0)
    {
        selectedRect.setHeight(1);
    }

    return selectedRect;
}

// 获取移动后,当前选中的矩形;
QRect imgCapWidget::getMoveRect()
{
    // 通过getMovePoint方法先检查当前是否移动超出屏幕;
    QPoint movePoint = getMovePoint();
    QPoint beginPoint = beginPoints + movePoint;
    QPoint endPoint = endPoints + movePoint;
    // 结束移动选区时更新当前beginPoints , endPoints,防止下一次操作时截取的图片有问题;
    if (currentCaptureState == FinishMoveCaptureArea)
    {
        beginPoints = beginPoint;
        endPoints = endPoint;
        beginMovePoint = QPoint(0, 0);
        endMovePoint = QPoint(0, 0);
    }
    return getRect(beginPoint, endPoint);
}

QPoint imgCapWidget::getMovePoint()
{
    QPoint movePoint = endMovePoint - beginMovePoint;
    QRect currentRect = getRect(beginPoints, endPoints);
    // 检查当前是否移动超出屏幕;

    //移动选区是否超出屏幕左边界;
    if (currentRect.topLeft().x() + movePoint.x() < 0)
    {
        movePoint.setX(0 - currentRect.topLeft().x());
    }
    //移动选区是否超出屏幕上边界;
    if (currentRect.topLeft().y() + movePoint.y() < 0)
    {
        movePoint.setY(0 - currentRect.topLeft().y());
    }
    //移动选区是否超出屏幕右边界;
    if (currentRect.bottomRight().x() + movePoint.x() > imageWidth)
    {
        movePoint.setX(imageWidth - currentRect.bottomRight().x());
    }
    //移动选区是否超出屏幕下边界;
    if (currentRect.bottomRight().y() + movePoint.y() > imageHeight)
    {
        movePoint.setY(imageHeight - currentRect.bottomRight().y());
    }

    return movePoint;
}

// 获取当前鼠标位于哪一个拖拽顶点;
StretchRectState imgCapWidget::getStrethRectState(QPoint point)
{
    StretchRectState stretchRectState = NotSelect;
    if (topLeftRect.contains(point))
    {
        stretchRectState = TopLeftRect;
    }
    else if (topCenterRect.contains(point))
    {
        stretchRectState = TopCenterRect;
    }
    else if (topRightRect.contains(point))
    {
        stretchRectState = TopRightRect;
    }
    else if (rightCenterRect.contains(point))
    {
        stretchRectState = RightCenterRect;
    }
    else if (bottomRightRect.contains(point))
    {
        stretchRectState = BottomRightRect;
    }
    else if (bottomCenterRect.contains(point))
    {
        stretchRectState = BottomCenterRect;
    }
    else if (bottomLeftRect.contains(point))
    {
        stretchRectState = BottomLeftRect;
    }
    else if (leftCenterRect.contains(point))
    {
        stretchRectState = LeftCenterRect;
    }

    return stretchRectState;
}

// 设置鼠标停在拖拽定点处的样式;
void imgCapWidget::setStretchCursorStyle(StretchRectState stretchRectState)
{
    switch (stretchRectState)
    {
    case NotSelect:
        setCursor(Qt::ArrowCursor);
        break;
    case TopLeftRect:
    case BottomRightRect:
        setCursor(Qt::SizeFDiagCursor);
        break;
    case TopRightRect:
    case BottomLeftRect:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case LeftCenterRect:
    case RightCenterRect:
        setCursor(Qt::SizeHorCursor);
        break;
    case TopCenterRect:
    case BottomCenterRect:
        setCursor(Qt::SizeVerCursor);
        break;
    default:
        break;
    }
}

// 获取拖拽后的矩形选中区域;
QRect imgCapWidget::getStretchRect()
{
    QRect stretchRect;
    QRect currentRect = getRect(beginPoints, endPoints);
    switch (stretchRectState)
    {
    case NotSelect:
        stretchRect = getRect(beginPoints, endPoints);
        break;
    case TopLeftRect:
        {
            stretchRect = getRect(currentRect.bottomRight(), endMovePoint);
        }
        break;
    case TopRightRect:
        {
            QPoint beginPoint = QPoint(currentRect.topLeft().x(), endMovePoint.y());
            QPoint endPoint = QPoint(endMovePoint.x(), currentRect.bottomRight().y());
            stretchRect = getRect(beginPoint, endPoint);
        }
        break;
    case BottomLeftRect:
        {
            QPoint beginPoint = QPoint(endMovePoint.x() , currentRect.topLeft().y());
            QPoint endPoint = QPoint(currentRect.bottomRight().x(), endMovePoint.y());
            stretchRect = getRect(beginPoint, endPoint);
        }
        break;
    case BottomRightRect:
        {
            stretchRect = getRect(currentRect.topLeft(), endMovePoint);
        }
        break;
    case LeftCenterRect:
        {
            QPoint beginPoint = QPoint(endMovePoint.x(), currentRect.topLeft().y());
            stretchRect = getRect(beginPoint, currentRect.bottomRight());
        }
        break;
    case TopCenterRect:
    {
        QPoint beginPoint = QPoint(currentRect.topLeft().x(), endMovePoint.y());
        stretchRect = getRect(beginPoint, currentRect.bottomRight());
    }
        break;
    case RightCenterRect:
    {
        QPoint endPoint = QPoint(endMovePoint.x(), currentRect.bottomRight().y());
        stretchRect = getRect(currentRect.topLeft(), endPoint);
    }
        break;
    case BottomCenterRect:
    {
        QPoint endPoint = QPoint(currentRect.bottomRight().x(), endMovePoint.y());
        stretchRect = getRect(currentRect.topLeft(), endPoint);
    }
        break;
    default:
    {
        stretchRect = getRect(beginPoints , endPoints );
    }
        break;
    } // 拖动结束更新 beginPoints , endPoints;
    if (currentCaptureState == FinishMoveStretchRect)
    {
        beginPoints = stretchRect.topLeft();
        endPoints = stretchRect.bottomRight();
    } return stretchRect;
}

// 绘制选中矩形信息,左上角;
void imgCapWidget::drawSelectRectInfo()
{
    int posX, posY;
    QPoint topLeftPoint = currentSelectRect.topLeft();

    posX = topLeftPoint.x() + SELECT_RECT_BORDER_WIDTH;
    if (topLeftPoint.y() > SELECT_RECT_INFO_HEIGHT)
    {
        posY = topLeftPoint.y() - SELECT_RECT_INFO_HEIGHT - SELECT_RECT_BORDER_WIDTH;
    }
    else
    {
        posY = topLeftPoint.y() + SELECT_RECT_BORDER_WIDTH;
    }
    topLeftPoint = QPoint(posX, posY);

    QColor backColor = QColor(0, 0, 0, 160);
    painter.fillRect(QRect(topLeftPoint, QSize(SELECT_RECT_INFO_WIDTH, SELECT_RECT_INFO_HEIGHT)), backColor);

    // 当前选中矩形的宽高信息;
    QString selectRectSizeInfo = QString("%1 * %2").arg(currentSelectRect.width()).arg(currentSelectRect.height());
    int fontWidth = this->fontMetrics().width(selectRectSizeInfo);
    painter.setPen(QPen(Qt::white));
    painter.drawText(QPoint(topLeftPoint.x() + (SELECT_RECT_INFO_WIDTH - fontWidth) / 2, topLeftPoint.y() + 14), selectRectSizeInfo);
}

void imgCapWidget::restartCapture()
{
    currentCaptureState = InitCapture;
    initStretchRect();
}
