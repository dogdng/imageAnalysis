#ifndef CHARTWIDGET_H
#define CHARTWIDGET_H

#include <iostream>
#include <vector>
#include <QWidget>
#include <QLineSeries>
#include <QtCharts/QChartView>
#include <QtCharts/QAreaSeries>
#include <QtCharts/QLegend>
#include <QtWidgets/QFormLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QApplication>
#include <QtCharts/QValueAxis>

using namespace std;
QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
class QChart;
QT_CHARTS_END_NAMESPACE

typedef vector<int> DataList;
typedef vector<DataList> DataTable;

QT_CHARTS_USE_NAMESPACE
class chartWidget: public QWidget
{
    Q_OBJECT
public:
    chartWidget(QWidget *parent = nullptr);
    ~chartWidget();

    void getData(DataTable data);
private:
    void setUI();
    QChart *createAreaChart() const;

    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QSpacerItem *horizontalSpacer;
    QChartView *chartView;
    DataTable dataTable;
    void initDataTable();
};

#endif // CHARTWIDGET_H
