#include "chartWidget.h"
#include <algorithm>

chartWidget::chartWidget(QWidget *parent)
    :QWidget(parent)
{
    gridLayout = new QGridLayout(this);
    //create charts
    initDataTable();
    chartView = new QChartView(createAreaChart());
    gridLayout->addWidget(chartView);

    setUI();
    chartView->hide();
}


chartWidget::~chartWidget()
{

}
void chartWidget::initDataTable()
{
    dataTable.emplace_back(0);
    dataTable.emplace_back(0);
    dataTable.emplace_back(0);
    for(int i(0); i<256; i++)
    {
        dataTable[0].emplace_back(0);
        dataTable[1].emplace_back(0);
        dataTable[2].emplace_back(0);
    }
}

void chartWidget::getData(DataTable data)//vector<int> RValues, vector<int> GValues, vector<int> BValues)
{
    dataTable.clear();
    dataTable = data;
    chartView->deleteLater();
    chartView = new QChartView(createAreaChart());
    gridLayout->addWidget(chartView);
    chartView->show();
}

QChart *chartWidget::createAreaChart() const
{
    QChart *chart = new QChart();
    chart->setTitle("histogram");

    // The lower series initialized to zero values
    QLineSeries *lowerSeries = nullptr;

    for (uint i(0); i < dataTable.size(); i++)
    {
        QLineSeries *upperSeries = new QLineSeries(chart);
        for (uint j(0); j < dataTable[i].size(); j++)
        {
            int data = dataTable[i].at(j);
            if (lowerSeries)
            {
                const QVector<QPointF>& points = lowerSeries->pointsVector();
                upperSeries->append(QPointF(j, points[i].y() + data));
            }
            else
            {
                upperSeries->append(QPointF(j, data));
            }
        }
        QAreaSeries *area = new QAreaSeries(upperSeries, lowerSeries);
        //QPen pen;
        switch(i)
        {
        case 0:
            area->setName(QString("red"));
            area->setColor(QColor(255,0,0,255));
            break;
        case 1:
            area->setName(QString("green"));
            area->setColor(QColor(0,255,0,255));
            break;
        case 2:
            //pen.setColor(QColor(0,0,255,255));
            area->setName(QString("blue"));
            area->setColor(QColor(0,0,255,255));
            //area->setPen(pen);
            break;
        default:
            break;
        }

        //area->useOpenGL();
        chart->addSeries(area);
        lowerSeries = upperSeries;
    }

    //由最大值确定Y轴的坐标范围
    auto maxItem0 = max_element(dataTable[0].begin(), dataTable[0].end());
    auto maxItem1 = max_element(dataTable[1].begin(), dataTable[1].end());
    auto maxItem2 = max_element(dataTable[2].begin(), dataTable[2].end());

    int maxItem=max(*maxItem0,*maxItem1);
    maxItem=max(maxItem, *maxItem2);

    chart->createDefaultAxes();
    chart->axisX()->setRange(0, 255);
    chart->axisY()->setRange(0, maxItem);
    // Add space to label to add space between labels and axis
    static_cast<QValueAxis *>(chart->axisY())->setLabelFormat("%d");//.1f  ");
    static_cast<QValueAxis *>(chart->axisX())->setLabelFormat("%d");//.1f  ");


    return chart;
}

void chartWidget::setUI()
{
//    QChart::ChartTheme theme = static_cast<QChart::ChartTheme>(QChart::ChartThemeDark);

//    chartView->chart()->setTheme(theme);
//    QPalette pal = window()->palette();

//    pal.setColor(QPalette::Window, QRgb(0x121218));
//    pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));

//    window()->setPalette(pal);
     // Update antialiasing

    chartView->setRenderHint(QPainter::Antialiasing, true);//抗锯齿


    // Update animation options
    QChart::AnimationOptions options(QChart::AllAnimations);
    if (chartView->chart()->animationOptions() != options)
    {
        chartView->chart()->setAnimationOptions(options);
    }

    // Update legend alignment
    Qt::Alignment alignment(Qt::AlignTop);

    if (!alignment)
    {
        chartView->chart()->legend()->hide();
    }
    else
    {
        chartView->chart()->legend()->setAlignment(alignment);
        chartView->chart()->legend()->show();
    }
}

