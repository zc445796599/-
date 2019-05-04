#include <QtGui>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QFileDevice>
#include <QTextStream>
#include <QtEvents>

MainWindow::MainWindow()
{
    setAttribute(Qt::WA_DeleteOnClose);

    textEdit = new QTextEdit;
    setCentralWidget(textEdit);

    createActions();
    createMenus();
    (void)statusBar();

    setWindowFilePath(QString());
    resize(800, 800);
}

void MainWindow::newFile()
{
    MainWindow *other = new MainWindow;
    other->show();
}

void MainWindow::open()
{
    QString fileName = QFileDialog::getOpenFileName(this,tr("打开"),tr(""),tr("Text File(*.txt)"));;//使用QFileDialog中的getOpenFileNmae打开文件
    if (!fileName.isEmpty())
        loadFile(fileName);
}

void MainWindow::save()
{
    if (curFile.isEmpty())
        saveAs();
    else
        saveFile(curFile);
}

void MainWindow::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this,tr("打开"),"",tr("Text File(*.txt)"));//使用getSaveFileName获得保存文件的字符串
    if (fileName.isEmpty())
        return;
    saveFile(fileName);
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
        loadFile(action->data().toString());
}



void MainWindow::createActions()
{
    newAct = new QAction(tr("新建"), this);
    newAct->setStatusTip(tr("创建文件"));
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));//使用connect将函数与action连接起来，并以触发器的形式调用函数

    openAct = new QAction(tr("打开"), this);
    openAct->setStatusTip(tr("打开文件"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("保存"), this);
    saveAct->setStatusTip(tr("保存"));
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("另存"), this);
    saveAsAct->setStatusTip(tr("另存为　"));
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = new QAction(this);
        recentFileActs[i]->setVisible(false);
        connect(recentFileActs[i], SIGNAL(triggered()),
            this, SLOT(openRecentFile()));
    }

    exitAct = new QAction(tr("关闭　"),this);
    exitAct->setStatusTip(tr("关闭"));
    connect(exitAct, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("文件"));
    fileMenu->addAction(newAct);
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    separatorAct = fileMenu->addSeparator();
    for (int i = 0; i < MaxRecentFiles; ++i)
        fileMenu->addAction(recentFileActs[i]);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);
    updateRecentFileActions();
    menuBar()->addSeparator();


}

void MainWindow::loadFile(const QString &fileName)//读取文件
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("最近使用的文件"),
            tr("无法读文件 %1:\n%2.")
            .arg(fileName)
            .arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);//
    textEdit->setPlainText(in.readAll());
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("文件读取完成"), 2000);
}

void MainWindow::saveFile(const QString &fileName)//保存文件
{
    QFile file(fileName);
    if (!file.open(QFile::WriteOnly | QFile::Text)) {
        QMessageBox::warning(this, tr("最近使用的文件"),
            tr("无法写文件 %1:\n%2.")
            .arg(fileName)
            .arg(file.errorString()));
        return;
    }

    QTextStream out(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    out << textEdit->toPlainText();
    QApplication::restoreOverrideCursor();

    setCurrentFile(fileName);
    statusBar()->showMessage(tr("文件保存完成"), 2000);
}

void MainWindow::setCurrentFile(const QString &fileName)//创建最近使用的文件
{
    curFile = fileName;
    setWindowFilePath(curFile);

    QSettings settings;
    QStringList files = settings.value("最近使用的文件列表").toStringList();
    files.removeAll(fileName);
    files.prepend(fileName);
    while (files.size() > MaxRecentFiles)
        files.removeLast();//当文件数大于5时，删除最后一个

    settings.setValue("最近使用的文件列表", files);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        MainWindow *mainWin = qobject_cast<MainWindow *>(widget);
        if (mainWin)
            mainWin->updateRecentFileActions();
    }
}

void MainWindow::updateRecentFileActions()//更新最近文件列表
{
    QSettings settings;
    QStringList files = settings.value("最近使用的文件列表").toStringList();

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);//取数量少的那一个并遍历

    for (int i = 0; i < numRecentFiles; ++i) {
        QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
        recentFileActs[i]->setText(text);
        recentFileActs[i]->setData(files[i]);
        recentFileActs[i]->setVisible(true);
    }
    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
        recentFileActs[j]->setVisible(false);

    separatorAct->setVisible(numRecentFiles > 0);
}

QString MainWindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QMessageBox::warning(this,tr("警告"),tr("确认退出？"),QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes;
    event->accept();
}
