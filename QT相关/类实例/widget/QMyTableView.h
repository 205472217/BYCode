#ifndef QMYTABLEVIEW_H
#define QMYTABLEVIEW_H

#include <QApplication>
#include <QMainWindow>
#include <QAbstractItemModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QTableView>
#include <QHeaderView>
#include <QMouseEvent>
#include <QPainter>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>

#define  DF_MAX_COL         (1024)
#define  DF_MAX_ROW         (1024*1024)

#define  CELL_WIDTH         (100)
#define  CELL_WIDTH_SMALL   (30)
#define  CELL_WIDTH_BIG     (150)

#define  CELL_HEIGHT        (25)
#define  CELL_HEIGHT_SMALL  (15)
#define  CELL_HEIGHT_BIG    (50)

class QMyTableView : public QTableView
{
    Q_OBJECT

public:
    QMyTableView(QWidget *parent = nullptr);
    void EnableMultiSelect(bool bEnable);
    void EnableDragItem(bool bEnable);
    void EnableSortRow(bool bEnable);

    int getColCount(){return dataModel->columnCount();};
    int getRowCount(){return dataModel->rowCount();}

    int addCol(const QString itemText, int colWidth = CELL_WIDTH, QHeaderView::ResizeMode = QHeaderView::Interactive);
    QString getColText(int colNum);

    int addRow(int rowHeight = CELL_HEIGHT);
    int insertRow(int npos = 0,int rowHeight = CELL_HEIGHT);
    int delRow(int rowNum);
    int delAllRow();
    int delAllRowCol();
    void setAllSectionState(bool bSelected);//设置所有行选中状态
    void setRowSectionState(int rowNum, bool bSelected);//设置指定行选中状态

    void setItemText(int rowNum, int colNum, const QString itemText);
    QString getItemText(int rowNum, int colNum);

    void setItemColor(int rowNum, int colNum, QColor clrText, QColor clrBack);
    void setItemColor(int rowNum, int colNum, QColor& clrText, QColor& clrBack);

    void setItemChecked(int rowNum, int colNum, bool checked);
    bool getItemChecked(int rowNum, int colNum);
    void setItemData(int nrow,int ncol,const QVariant &roles);
    QVariant getItemData(int nrow,int ncol);

    void setItemWidget(int rowNum, int colNum, QWidget* pWidget);
    QWidget* getItemWidget(int rowNum, int colNum);

    void setItemEdit(int rowNum, int colNum, const QString itemText = "");
    void setItemButton(int rowNum, int colNum, const QString itemText = "");
    void setItemComboBox(int rowNum, int colNum, const QStringList itemTextList, const QString selItemText);
    void setItemCheckBox(int rowNum, int colNum, const QString itemText, const bool checked = false);

    void enableAllWidget(bool bEnable);
    void enableRowWidget(int rowNum, bool bEnable);
    void enableItemWidget(int rowNum, int colNum, bool bEnable);

signals:
    void lostFocus();
    void selHSectionClicked(int colNum);
    void selHSectionDbClicked(int colNum);
    void selHSectionRightClicked(int colNum);
    void selVSectionClicked(int rowNum);
    void selVSectionDbClicked(int rowNum);
    void selVSectionRightClicked(int colNum);
    void cellClicked(int rowNum, int colNum);
    void cellDbClicked(int rowNum, int colNum);
    void buttonClicked(int rowNum, int colNum);
    void checkBoxClicked(int row, int column, bool checked);
    void dragRowItemData(int nSrcRow, int nDstRow);

public slots:
    void handleHSectionClicked(int nSectionIdx);
    void handleHSectionDbClicked(int nSectionIdx);
    void handleHSectionRClicked(const QPoint &pos);
    void handleVSectionClicked(int nSectionIdx);
    void handleVSectionDbClicked(int nSectionIdx);
    void handleVSectionRClicked(const QPoint &pos);

    void handlePushButtonClicked();
    void handleLineEditTextChanged(const QString& text);
    void handleLineEditEditingFinished();
    void handleComboBoxCurrentIndexChanged(int index);
    void handleCheckBoxCurrentIndexChanged(bool checked);

protected:
    void focusOutEvent(QFocusEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;

protected:
    int     m_colCount;
    int     m_rowCount;
    int     m_curSelectedRow;
    int     m_curSelectedCol;
    bool    m_widgetEnableArray[DF_MAX_ROW][DF_MAX_COL];

    int     m_rowHightArray[DF_MAX_ROW];
    int     m_colWidthArray[DF_MAX_COL];

    //Sort Row
    bool    m_bIsASC;                    //当前是否升序
    int     m_lastDbClickCol;            //最后一次双击的列
    bool    m_bEnableSortRow;
    bool    m_bEnableSortCol[DF_MAX_COL];//列中含有控件，则不支持排序

private:
    QStandardItemModel      *dataModel;
    QSortFilterProxyModel   *sortProxyModel;
};




#endif // QMYTABLEVIEW_H
