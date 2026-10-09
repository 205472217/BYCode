#include "QMyTableView.h"
#include <QDebug>

QMyTableView::QMyTableView(QWidget *parent)
    : QTableView(parent)
{
    Qt::Alignment alignment = Qt::AlignCenter; //居中对齐

    QHeaderView *pHorizontalHeader = horizontalHeader();
    pHorizontalHeader->setDefaultAlignment(alignment);
    pHorizontalHeader->setVisible(true);
    pHorizontalHeader->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(pHorizontalHeader, &QHeaderView::sectionClicked, this, &QMyTableView::handleHSectionClicked);
    QObject::connect(pHorizontalHeader, &QHeaderView::sectionDoubleClicked, this, &QMyTableView::handleHSectionDbClicked);
    QObject::connect(pHorizontalHeader, &QHeaderView::customContextMenuRequested, this, &QMyTableView::handleHSectionRClicked);

    QHeaderView *pVerticalHeader = verticalHeader();
    pVerticalHeader->setDefaultAlignment(alignment);
    pVerticalHeader->setVisible(false);
    pVerticalHeader->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(pVerticalHeader, &QHeaderView::sectionClicked, this, &QMyTableView::handleVSectionClicked);
    QObject::connect(pVerticalHeader, &QHeaderView::sectionDoubleClicked, this, &QMyTableView::handleVSectionDbClicked);
    QObject::connect(pVerticalHeader, &QHeaderView::customContextMenuRequested, this, &QMyTableView::handleVSectionRClicked);

    //创建模型
    dataModel = new QStandardItemModel(this);
    setModel(dataModel);

    setEditTriggers(QAbstractItemView::NoEditTriggers);  // 设置表格不可编辑
    setSelectionBehavior(QAbstractItemView::SelectRows); // 整行选中
    EnableMultiSelect(false);
    EnableDragItem(false);

    m_colCount = 0;
    m_rowCount = 0;
    m_curSelectedRow = -1;
    memset(m_rowHightArray,0,sizeof(int)*DF_MAX_ROW);
    memset(m_colWidthArray,0,sizeof(int)*DF_MAX_COL);

    //排序功能
    m_bEnableSortRow = false;
    for (int i = 0; i < DF_MAX_COL; i++)
    {
        m_bEnableSortCol[i] = false;
    }
}

void QMyTableView::EnableMultiSelect(bool bEnable)
{
    if (bEnable)
        setSelectionMode(QAbstractItemView::ExtendedSelection);// Ctrl多行选中
    else
        setSelectionMode(QAbstractItemView::SingleSelection);// 单行选中
}
void QMyTableView::EnableDragItem(bool bEnable)
{
    setDragEnabled(bEnable);
    setAcceptDrops(bEnable);
}
void QMyTableView::EnableSortRow(bool bEnable)
{
    m_bIsASC = true;
    m_lastDbClickCol = -1;
    m_bEnableSortRow = bEnable;
    for (int i = 0; i < DF_MAX_COL; i++)
    {
        m_bEnableSortCol[i] = bEnable;
    }
    sortProxyModel = new QSortFilterProxyModel(this);
    sortProxyModel->setSourceModel(dataModel);
    setModel(sortProxyModel);
}

int QMyTableView::addCol(const QString itemText, int colWidth, QHeaderView::ResizeMode nMode)
{
    int colCount = dataModel->columnCount();
    int rowCount = dataModel->rowCount();
    if (colCount >= DF_MAX_COL) return -1;
    if (rowCount >= DF_MAX_ROW) return -1;

    //记录列宽
    colWidth = fmax(CELL_WIDTH_SMALL,colWidth);
    m_colWidthArray[colCount] = colWidth;

    //添加表头
    dataModel->setHorizontalHeaderItem(colCount, new QStandardItem(itemText));
    if (nMode < QHeaderView::Interactive || nMode > QHeaderView::ResizeToContents)
        nMode = QHeaderView::Interactive;
    horizontalHeader()->setSectionResizeMode(colCount, nMode);

    //记录当前列数
    m_colCount = dataModel->columnCount();
    return m_colCount-1;
}
QString QMyTableView::getColText(int colNum)
{
    if (colNum >= m_colCount)
        return QString();

    QVariant headerData = dataModel->headerData(colNum, Qt::Horizontal);
    QString strColName = headerData.toString();

    return strColName;
}
int QMyTableView::insertRow(int npos,int rowHeight)
{
    int colCount = dataModel->columnCount();
    int rowCount = dataModel->rowCount();
    if (rowCount+1 >= DF_MAX_ROW) return -1;
    if(npos>rowCount) return -1;
    if(npos<0) return -1;

    //记录行高
    rowHeight = fmax(CELL_HEIGHT_SMALL,rowHeight);

    if(dataModel->insertRow(npos))
    {        //添加一行
        for(int i = 0; i < colCount; i++)
        {
            QStandardItem *item = new QStandardItem(QString(""));
            item->setTextAlignment(Qt::AlignCenter); //居中对齐
            dataModel->setItem(0, i, item);
        }
        for(int j= DF_MAX_ROW;j>npos+1;j--)
        {
            m_rowHightArray[j] = m_rowHightArray[j-1];
        }
        m_rowHightArray[0] = rowHeight;
        m_rowCount = dataModel->rowCount();
        return npos;
    }
    return -1;
}
int QMyTableView::addRow(int rowHeight)
{
    int colCount = dataModel->columnCount();
    int rowCount = dataModel->rowCount();
    if (colCount >= DF_MAX_COL) return -1;
    if (rowCount >= DF_MAX_ROW) return -1;

    //记录行高
    rowHeight = fmax(CELL_HEIGHT_SMALL,rowHeight);
    m_rowHightArray[rowCount] = rowHeight;

    //添加一行
    for(int i = 0; i < colCount; i++)
    {
        QStandardItem *item = new QStandardItem(QString(""));
        item->setTextAlignment(Qt::AlignCenter); //居中对齐
        dataModel->setItem(rowCount, i, item);
    }
    //记录当前行数
    m_rowCount = dataModel->rowCount();
    return m_rowCount-1;
}
int QMyTableView::delRow(int rowNum)
{
    if (!dataModel) return -1;
    int rowCount = dataModel->rowCount();
    int colCount = dataModel->columnCount();
    if (rowCount <= 0 || colCount <= 0)
        return -1;

    //删除行中的控件
    for (int colNum = 0; colNum < colCount; colNum++)
    {
        QWidget *pWidget = indexWidget(dataModel->index(rowNum, colNum));
        if(pWidget != NULL)
        {
            delete pWidget;
            pWidget =  NULL;
        }
    }

    //删除行
    dataModel->removeRow(rowNum);

    m_rowCount = dataModel->rowCount();
    return m_rowCount;
}
int QMyTableView::delAllRow()
{
    if (!dataModel) return -1;
    int rowCount = dataModel->rowCount();
    int colCount = dataModel->columnCount();
    if (rowCount <= 0 || colCount <= 0)
        return -1;

    //删除行中的控件
    for (int rowNum = 0; rowNum < rowCount; rowNum++)
    {
        for (int colNum = 0; colNum < colCount; colNum++)
        {
            QWidget *pWidget = indexWidget(dataModel->index(rowNum, colNum));
            if(pWidget != nullptr)
            {
                pWidget->deleteLater();
            }
        }
    }

    //删除所有行
    dataModel->removeRows(0, dataModel->rowCount());

    m_rowCount = 0;
    return m_rowCount;
}
int QMyTableView::delAllRowCol()
{
    dataModel = new QStandardItemModel(this);
    setModel(dataModel);

    m_colCount = 0;
    m_rowCount = 0;

    return 0;
}
void QMyTableView::setAllSectionState(bool bSelected)
{
    QModelIndexList indexes;
    for (int row = 0; row < model()->rowCount(); ++row) {
        for (int col = 0; col < model()->columnCount(); ++col) {
            indexes.append(model()->index(row, col));
        }
    }

    QItemSelectionModel *selecModel = selectionModel();
    if (bSelected)
    {
        QItemSelection selection(indexes.begin(), indexes.end());
        selecModel->select(selection, QItemSelectionModel::Select);
    }
    else
    {
        QItemSelection selection(indexes.begin(), indexes.end());
        selecModel->select(selection, QItemSelectionModel::Deselect);
    }
}
void QMyTableView::setRowSectionState(int rowNum, bool bSelected)
{
    if (rowNum < 0 || rowNum >= model()->rowCount())
        return;

    QModelIndexList indexes;
    for (int col = 0; col < model()->columnCount(); ++col) {
        indexes.append(model()->index(rowNum, col));
    }

    QItemSelectionModel *selecModel = selectionModel();
    if (bSelected)
    {
        QItemSelection selection(indexes.begin(), indexes.end());
        selecModel->select(selection, QItemSelectionModel::Select);
    }
    else
    {
        QItemSelection selection(indexes.begin(), indexes.end());
        selecModel->select(selection, QItemSelectionModel::Deselect);
    }
}

void QMyTableView::setItemData(int nrow,int ncol,const QVariant &roles)
{
    //Please Use QVariant::fromValue(xxx) Set
    //MyClass obj; setItemData(0, 0, QVariant::fromValue(&obj));
    QModelIndex index = dataModel->index(nrow,ncol);
    if(index.isValid())
    {
        dataModel->itemFromIndex(index)->setData(roles,Qt::UserRole);
    }
}
QVariant QMyTableView::getItemData(int nrow,int ncol)
{
    //Please Use QVariant's value<xxx's Type> Get
    //MyClass* pObj = getItemData(0,0).value<MyClass*>();
    QModelIndex index = dataModel->index(nrow,ncol);
    if(index.isValid())
    {
       return index.data(Qt::UserRole);
    }
    return 0;
}
void QMyTableView::setItemText(int rowNum, int colNum, const QString itemText)
{
    if (rowNum>DF_MAX_ROW || colNum>DF_MAX_COL)
        return;
    int colCount = dataModel->columnCount();
    int rowCount = dataModel->rowCount();
    if (rowNum >= rowCount) return;
    if (colNum >= colCount) return;

    QStandardItem* item = dataModel->itemFromIndex(dataModel->index(rowNum, colNum));
    if (item)
        item->setData(itemText, Qt::DisplayRole);
    else
        dataModel->setItem(rowNum, colNum, new QStandardItem(itemText));
    setRowHeight(rowNum,fmax(m_rowHightArray[rowNum],CELL_HEIGHT_SMALL));
    setColumnWidth(colNum,fmax(m_colWidthArray[colNum],CELL_WIDTH_SMALL));

    //指定行列控件文本
    QWidget *pWidget = indexWidget(dataModel->index(rowNum, colNum));
    if(pWidget != nullptr)
    {
        QLineEdit *pEdit = qobject_cast<QLineEdit*>(pWidget);
        if (pEdit) {
            pEdit->setText(itemText);
        }

        QPushButton *pButton = qobject_cast<QPushButton*>(pWidget);
        if (pButton) {
            pButton->setText(itemText);
        }

        QComboBox *pCombox = qobject_cast<QComboBox*>(pWidget);
        if (pCombox) {
            pCombox->setCurrentText(itemText);
        }

        QCheckBox *pCheckBox = qobject_cast<QCheckBox*>(pWidget);
        if (pCheckBox) {
            pCheckBox->setText(itemText);
        }
    }
}
QString QMyTableView::getItemText(int rowNum, int colNum)
{
    QString itemText = "";
    QWidget *pWidget = indexWidget(dataModel->index(rowNum, colNum));
    if (pWidget == nullptr)
    {
        QVariant data = dataModel->data(dataModel->index(rowNum, colNum));
        itemText = data.toString();
    }
    else
    {
        QLineEdit *pEdit = qobject_cast<QLineEdit*>(pWidget);
        if (pEdit) {
            itemText = pEdit->text();
        }

        QPushButton *pButton = qobject_cast<QPushButton*>(pWidget);
        if (pButton) {
            itemText = pButton->text();
        }

        QComboBox *pCombox = qobject_cast<QComboBox*>(pWidget);
        if (pCombox) {
            itemText = pCombox->currentText();
        }

        QCheckBox *pCheckBox = qobject_cast<QCheckBox*>(pWidget);
        if (pCheckBox) {
            itemText = pCheckBox->text();
        }
    }

    return itemText;
}

void QMyTableView::setItemColor(int rowNum, int colNum, QColor clrText, QColor clrBack)
{
    if (rowNum>DF_MAX_ROW || colNum>DF_MAX_COL)
        return;
    int colCount = dataModel->columnCount();
    int rowCount = dataModel->rowCount();
    if (rowNum >= rowCount) return;
    if (colNum >= colCount) return;

    // 获取单元格项
    QStandardItem *item = dataModel->item(rowNum, colNum);
    if (item)
    {
        // 设置单元格颜色
        QBrush brushText(clrText);
        item->setForeground(brushText);
        QBrush brushBack(clrBack);
        item->setBackground(brushBack);
    }
}
void QMyTableView::setItemColor(int rowNum, int colNum, QColor& clrText, QColor& clrBack)
{
    if (rowNum>DF_MAX_ROW || colNum>DF_MAX_COL)
        return;
    int colCount = dataModel->columnCount();
    int rowCount = dataModel->rowCount();
    if (rowNum >= rowCount) return;
    if (colNum >= colCount) return;

    // 获取单元格项
    QStandardItem *item = dataModel->item(rowNum, colNum);
    if (item)
    {
        clrText = item->foreground().color();
        clrBack = item->background().color();
    }
}

void QMyTableView::setItemChecked(int rowNum, int colNum, bool checked)
{
    QWidget *pWidget = indexWidget(dataModel->index(rowNum, colNum));
    if (pWidget != nullptr)
    {
        QCheckBox *pCheckBox = qobject_cast<QCheckBox*>(pWidget);
        if (pCheckBox) {
            pCheckBox->setChecked(checked);
        }
    }
}
bool QMyTableView::getItemChecked(int rowNum, int colNum)
{
    bool checked = false;
    QWidget *pWidget = indexWidget(dataModel->index(rowNum, colNum));
    if (pWidget != nullptr)
    {
        QCheckBox *pCheckBox = qobject_cast<QCheckBox*>(pWidget);
        if (pCheckBox) {
            checked = pCheckBox->isChecked();
        }
    }

    return checked;
}

void QMyTableView::setItemWidget(int rowNum, int colNum, QWidget* pWidget)
{
    if (rowNum>DF_MAX_ROW || colNum>DF_MAX_COL)
        return;
    m_bEnableSortCol[colNum] = false;

    if (pWidget == nullptr)
    {
        setItemText(rowNum,colNum,QString(""));
        return;
    }
    else
    {
        pWidget->setBaseSize(columnWidth(colNum)-1,rowHeight(rowNum)-1);
        pWidget->setEnabled(m_widgetEnableArray[rowNum][colNum]);

        //插入到指定单元格
        setIndexWidget(dataModel->index(rowNum, colNum), pWidget);
        setRowHeight(rowNum,fmax(m_rowHightArray[rowNum],CELL_HEIGHT_SMALL));
        setColumnWidth(colNum,fmax(m_colWidthArray[colNum],CELL_WIDTH_SMALL));

        //连接信号到槽函数
        QPushButton *pButton = dynamic_cast<QPushButton*>(pWidget);
        if (pButton != nullptr)
        {
            QObject::connect(pButton, (&QPushButton::clicked), this, (&QMyTableView::handlePushButtonClicked));
        }
    }
}
QWidget* QMyTableView::getItemWidget(int rowNum, int colNum)
{
    if (rowNum>DF_MAX_ROW || colNum>DF_MAX_COL)
        return nullptr;

    return indexWidget(dataModel->index(rowNum, colNum));
}

void QMyTableView::setItemEdit(int rowNum, int colNum, const QString itemText)
{
    if (rowNum>DF_MAX_ROW || colNum>DF_MAX_COL)
        return;
    m_bEnableSortCol[colNum] = false;

    //创建控件
    QLineEdit *pEdit = new QLineEdit(itemText, this);
    pEdit->setBaseSize(columnWidth(colNum)-1,rowHeight(rowNum)-1);
    pEdit->setEnabled(m_widgetEnableArray[rowNum][colNum]);

    //插入到指定单元格
    setIndexWidget(dataModel->index(rowNum, colNum), pEdit);
    setRowHeight(rowNum,fmax(m_rowHightArray[rowNum],CELL_HEIGHT_SMALL));
    setColumnWidth(colNum,fmax(m_colWidthArray[colNum],CELL_WIDTH_SMALL));

    //添加槽关联
    QObject::connect(pEdit, (&QLineEdit::textChanged), this, (&QMyTableView::handleLineEditTextChanged));
    QObject::connect(pEdit, (&QLineEdit::editingFinished), this, (&QMyTableView::handleLineEditEditingFinished));
}
void QMyTableView::setItemButton(int rowNum, int colNum, const QString itemText)
{
    if (rowNum>DF_MAX_ROW || colNum>DF_MAX_COL)
        return;
    m_bEnableSortCol[colNum] = false;

    //创建控件
    QPushButton *pButton = new QPushButton(itemText, this);
    pButton->setBaseSize(columnWidth(colNum)-1,rowHeight(rowNum)-1);
    pButton->setEnabled(m_widgetEnableArray[rowNum][colNum]);

    //将插入到指定单元格
    setIndexWidget(dataModel->index(rowNum, colNum), pButton);
    setRowHeight(rowNum,fmax(m_rowHightArray[rowNum],CELL_HEIGHT_SMALL));
    setColumnWidth(colNum,fmax(m_colWidthArray[colNum],CELL_WIDTH_SMALL));

    //连接按钮的点击信号到槽函数
    QObject::connect(pButton, (&QPushButton::clicked), this, (&QMyTableView::handlePushButtonClicked));
}
void QMyTableView::setItemComboBox(int rowNum, int colNum, const QStringList itemTextList, const QString selItemText)
{
    if (rowNum>DF_MAX_ROW || colNum>DF_MAX_COL)
        return;
    m_bEnableSortCol[colNum] = false;

    //创建控件
    QComboBox *pComboBox = new QComboBox(this);
    pComboBox->setBaseSize(columnWidth(colNum)-1,rowHeight(rowNum)-1);
    pComboBox->setEnabled(m_widgetEnableArray[rowNum][colNum]);

    int selItemIdx = -1;
    int itemCount = itemTextList.count();
    for(int i = 0; i < itemCount; i++)
    {
        QString text = itemTextList.at(i);
        pComboBox->addItem(text);
        if (selItemText == text)
        {
            selItemIdx = i;
        }
    }

    //插入到指定单元格
    pComboBox->setCurrentIndex(selItemIdx);
    setIndexWidget(dataModel->index(rowNum, colNum), pComboBox);
    setRowHeight(rowNum,fmax(m_rowHightArray[rowNum],CELL_HEIGHT_SMALL));
    setColumnWidth(colNum,fmax(m_colWidthArray[colNum],CELL_WIDTH_SMALL));
}
void QMyTableView::setItemCheckBox(int rowNum, int colNum, const QString itemText, const bool checked)
{
    if (rowNum>DF_MAX_ROW || colNum>DF_MAX_COL)
        return;
    m_bEnableSortCol[colNum] = false;

    //创建控件
    QCheckBox *pCheckBox = new QCheckBox(this);
    pCheckBox->setBaseSize(columnWidth(colNum)-1,rowHeight(rowNum)-1);
    pCheckBox->setText(itemText);
    pCheckBox->setChecked(checked);
    pCheckBox->setEnabled(m_widgetEnableArray[rowNum][colNum]);

    //插入到指定单元格
    setIndexWidget(dataModel->index(rowNum, colNum), pCheckBox);
    setRowHeight(rowNum,fmax(m_rowHightArray[rowNum],CELL_HEIGHT_SMALL));
    setColumnWidth(colNum,fmax(m_colWidthArray[colNum],CELL_WIDTH_SMALL));

    //连接勾选框的点击信号到槽函数
    QObject::connect(pCheckBox, (&QCheckBox::clicked), this, (&QMyTableView::handleCheckBoxCurrentIndexChanged));
}

void QMyTableView::enableAllWidget(bool bEnable)
{
    for (int i = 0; i < m_rowCount; i++)
    {
        for (int j = 0; j < m_colCount; j++)
        {
            m_widgetEnableArray[i][j] = bEnable;
            enableItemWidget(i, j, bEnable);
        }
    }
}
void QMyTableView::enableRowWidget(int rowNum, bool bEnable)
{
    if (rowNum >= m_rowCount) return;

    for (int j = 0; j < m_colCount; j++)
    {
        m_widgetEnableArray[rowNum][j] = bEnable;
        enableItemWidget(rowNum, j, bEnable);
    }
}
void QMyTableView::enableItemWidget(int rowNum, int colNum, bool bEnable)
{
    if (rowNum >= m_rowCount) return;
    if (colNum >= m_colCount) return;

    QWidget *pWidget = indexWidget(dataModel->index(rowNum, colNum));
    if (pWidget != nullptr)
    {
        m_widgetEnableArray[rowNum][colNum] = bEnable;
        pWidget->setEnabled(bEnable);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void QMyTableView::handleHSectionClicked(int nSectionIdx)
{
    qDebug() << "Horizontal Clicked: SectionIdx = " << nSectionIdx;

    emit selHSectionClicked(nSectionIdx);
}
void QMyTableView::handleHSectionDbClicked(int nSectionIdx)
{
    qDebug() << "Horizontal DbClicked: SectionIdx = " << nSectionIdx;

    if (nSectionIdx >= 0)
    {
        // 双击在表头
        if (m_bEnableSortRow && m_bEnableSortCol[nSectionIdx])
        {
            if (m_lastDbClickCol == nSectionIdx)
            {
                m_bIsASC = !m_bIsASC;
            }
            else
            {
                m_bIsASC = true;
                m_lastDbClickCol = nSectionIdx;
            }

            // 排序模型
            sortProxyModel->sort(m_lastDbClickCol, m_bIsASC ? Qt::AscendingOrder : Qt::DescendingOrder);
        }
    }

    emit selHSectionDbClicked(nSectionIdx);
}
void QMyTableView::handleHSectionRClicked(const QPoint &pos)
{
    // 获取点击的位置对应的section索引
    int sectionIndex = horizontalHeader()->logicalIndexAt(pos);
    qDebug() << "Horizontal Right clicked: SectionIdx = " << sectionIndex;

    emit selHSectionRightClicked(sectionIndex);
}
void QMyTableView::handleVSectionClicked(int nSectionIdx)
{
    qDebug() << "Vertical Clicked: SectionIdx = " << nSectionIdx;

    emit selVSectionClicked(nSectionIdx);
}
void QMyTableView::handleVSectionDbClicked(int nSectionIdx)
{
    qDebug() << "Vertical DbClicked: SectionIdx = " << nSectionIdx;

    emit selVSectionDbClicked(nSectionIdx);
}
void QMyTableView::handleVSectionRClicked(const QPoint &pos)
{
    // 获取点击的位置对应的section索引
    int sectionIndex = horizontalHeader()->logicalIndexAt(pos);
    qDebug() << "Vertical Right clicked: SectionIdx = " << sectionIndex;

    emit selVSectionRightClicked(sectionIndex);
}

void QMyTableView::handlePushButtonClicked()
{
    QPushButton *pushButton = qobject_cast<QPushButton *>(sender());
    if (pushButton)
    {
        int x = pushButton->mapToParent(QPoint(0,0)).x();
        int y = pushButton->mapToParent(QPoint(0,0)).y();
        QModelIndex index = this->indexAt(QPoint(x,y));
        int row = index.row();
        int col = index.column();

        emit buttonClicked(row,col);
    }
}
void QMyTableView::handleLineEditTextChanged(const QString& text)
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (lineEdit)
    {
        int x = lineEdit->mapToParent(QPoint(0,0)).x();
        int y = lineEdit->mapToParent(QPoint(0,0)).y();
        QModelIndex index = this->indexAt(QPoint(x,y));
        int row = index.row();
        int col = index.column();

        QStandardItem* item = dataModel->itemFromIndex(dataModel->index(row, col));
        if (item)
            item->setData(text, Qt::DisplayRole);
        else
            dataModel->setItem(row, col, new QStandardItem(text));
    }
}
void QMyTableView::handleLineEditEditingFinished()
{
    QLineEdit *lineEdit = qobject_cast<QLineEdit *>(sender());
    if (lineEdit)
    {
        int x = lineEdit->mapToParent(QPoint(0,0)).x();
        int y = lineEdit->mapToParent(QPoint(0,0)).y();
        QModelIndex index = this->indexAt(QPoint(x,y));
        int row = index.row();
        int col = index.column();

        QString lineEditText = lineEdit->text();

        QStandardItem* item = dataModel->itemFromIndex(dataModel->index(row, col));
        if (item)
            item->setData(lineEditText, Qt::DisplayRole);
        else
            dataModel->setItem(row, col, new QStandardItem(lineEditText));
    }
}
void QMyTableView::handleComboBoxCurrentIndexChanged(int index)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox)
    {
        int x = comboBox->mapToParent(QPoint(0,0)).x();
        int y = comboBox->mapToParent(QPoint(0,0)).y();
        QModelIndex index = this->indexAt(QPoint(x,y));
        int row = index.row();
        int col = index.column();

        QString comboBoxText = comboBox->currentText();
        QStandardItem* item = dataModel->itemFromIndex(dataModel->index(row, col));
        if (item)
            item->setData(comboBoxText, Qt::DisplayRole);
        else
            dataModel->setItem(row, col, new QStandardItem(comboBoxText));
    }
}
void QMyTableView::handleCheckBoxCurrentIndexChanged(bool checked)
{
    QCheckBox *checkBox = qobject_cast<QCheckBox *>(sender());
    if (checkBox)
    {
        int x = checkBox->mapToParent(QPoint(0,0)).x();
        int y = checkBox->mapToParent(QPoint(0,0)).y();
        QModelIndex index = this->indexAt(QPoint(x,y));
        int row = index.row();
        int col = index.column();

        emit checkBoxClicked(row,col,checked);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////
void QMyTableView::focusOutEvent(QFocusEvent *event)
{
    //调用基类的 focusOutEvent 函数以确保正确处理焦点事件
    QTableView::focusOutEvent(event);

    emit lostFocus(); //发出自定义信号，表示控件失去焦点
}
void QMyTableView::mousePressEvent(QMouseEvent *event)
{
    QTableView::mousePressEvent(event);

    QModelIndex index = indexAt(event->pos());
    if (index.isValid()) 
    {
        m_curSelectedRow = index.row();
        m_curSelectedCol = index.column();
        emit cellClicked(m_curSelectedRow, m_curSelectedCol);
        qDebug() << "QMyTableView clicked: Row=" << m_curSelectedRow << ",Col=" << m_curSelectedCol;
    }
}
void QMyTableView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QModelIndex index = indexAt(event->pos());
    if (index.isValid())
    {
        m_curSelectedRow = index.row();
        m_curSelectedCol = index.column();
        emit cellDbClicked(m_curSelectedRow, m_curSelectedCol);
        qDebug() << "QMyTableView DbClicked: Row=" << m_curSelectedRow << ",Col=" << m_curSelectedCol;
    }

    return QTableView::mouseDoubleClickEvent(event);
}
void QMyTableView::paintEvent(QPaintEvent *event)
{
    QTableView::paintEvent(event);
}
void QMyTableView::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
}
void QMyTableView::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->source() == this) {
        event->acceptProposedAction();
    }
}
void QMyTableView::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->source() == this) {
        event->acceptProposedAction();
    }
}
void QMyTableView::dropEvent(QDropEvent *event)
{
    if (event->source() == this)
    {
        int nSrcRow = m_curSelectedRow;
        int nDstRow = indexAt(event->pos()).row();
        qDebug() << "nSrcRow = " << nSrcRow;
        qDebug() << "nDstRow = " << nDstRow;

        if (nSrcRow!=nDstRow && nSrcRow>=0 && nDstRow>=0 && nSrcRow<m_rowCount && nDstRow<m_rowCount)
        {
            //向外发送信号,外部进行处理
            emit dragRowItemData(nSrcRow,nDstRow);
        }
        event->accept();
    }
}
