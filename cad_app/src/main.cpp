/**
 * @file main.cpp
 * @brief Ander CAD的主程序入口 - 一切故事开始的地方！
 * 
 * 这里是整个CAD应用程序的"大门"，负责初始化所有必要的组件，
 * 设置应用程序属性，创建主窗口，然后把控制权交给Qt的事件循环。
 * 
 * 就像开一家店一样：先开门，检查设备，摆好货架，然后等客人来 🏪
 * 
 * TODO: 添加命令行参数解析功能
 * TODO: 实现单实例检查，避免重复启动
 * TODO: 添加错误日志记录功能
 */

#include <QApplication>          // Qt应用程序的"心脏"
#include <QMessageBox>           // 消息对话框 - 和用户聊天的工具
#include <QStyleFactory>         // 样式工厂 - 让界面更美观
#include <QDir>                  // 目录操作 - 文件系统的导航员
#include <QStandardPaths>        // 标准路径 - 系统文件夹的GPS
#include <QSettings>             // 设置管理 - 记住用户的偏好
#include <QSplashScreen>         // 启动画面 - 第一印象很重要
#include <QPixmap>               // 像素图 - 图片的基本单位
#include <QTimer>                // 定时器 - 时间管理大师

#include "cad_ui/MainWindow.h"   // 我们的主窗口 - 用户界面的"指挥中心"

// QRC资源初始化函数声明 - 手动初始化静态库中的资源
extern int qInitResources_resources();

// OpenCASCADE的初始化相关头文件 - 让几何计算引擎苏醒
#include <Standard_Version.hxx>      // 版本信息 - 知己知彼
#include <Message.hxx>               // 消息系统 - OpenCASCADE的"嘴巴"
#include <Message_PrinterOStream.hxx> // 输出流打印器 - 把消息送到控制台

/**
 * 主函数 - 程序的"总指挥官"
 * @param argc 命令行参数数量
 * @param argv 命令行参数数组
 * @return 程序退出码，0表示成功，其他值表示出错了
 */
int main(int argc, char *argv[])
{
    // 创建Qt应用程序对象 - 这是一切的开始！
    QApplication app(argc, argv);
    
    // 手动初始化QRC资源 - 确保静态库中的资源能被正确加载
    qInitResources_resources();
    
    // 设置应用程序属性 - 给我们的程序一个"身份证"
    app.setApplicationName("Ander CAD");        // 应用名称 - 我们的"艺名"
    app.setApplicationVersion("1.0.0");         // 版本号 - 我们现在还是"新生儿"
    app.setOrganizationName("Ander CAD");       // 组织名称 - 我们的"厂牌"
    app.setOrganizationDomain("andercad.com");  // 域名 - 我们的"网络地址"
    
    // 设置应用程序图标（如果有的话）
    // TODO: 设计一个酷炫的图标，让用户一眼就能认出我们
    // app.setWindowIcon(QIcon(":/icons/app-icon.png"));
    
    // 初始化OpenCASCADE消息系统 - 让几何引擎能够"说话"
    // 这样我们就能知道OpenCASCADE在干什么，出了什么问题
    Handle(Message_PrinterOStream) printer = new Message_PrinterOStream();
    Message::DefaultMessenger()->AddPrinter(printer);
    
    // 创建启动画面（可选功能）- 给用户一个"正在加载"的安全感
    QSplashScreen* splash = nullptr;
    /*
    // TODO: 设计一个炫酷的启动画面，让等待变得有趣
    QPixmap splashPixmap(":/images/splash.png");
    if (!splashPixmap.isNull()) {
        splash = new QSplashScreen(splashPixmap);
        splash->show();
        splash->showMessage("正在启动 Ander CAD...", Qt::AlignBottom | Qt::AlignCenter, Qt::white);
        app.processEvents();  // 让界面响应一下，不要让用户以为程序卡死了
    }
    */
    
    // 创建主窗口 - 我们程序的"脸面"
    cad_ui::MainWindow mainWindow;
    
    // 初始化主窗口 - 这一步很重要，相当于"开机自检"
    if (!mainWindow.Initialize()) {
        // 如果初始化失败，礼貌地告诉用户发生了什么
        // 而不是神秘地崩溃，让用户摸不着头脑
        QMessageBox::critical(nullptr, "初始化失败", 
            "抱歉，应用程序初始化失败了 😢\n\n"
            "请检查以下项目：\n"
            "• OpenCASCADE 7.8 是否正确安装\n"
            "• 显卡驱动是否是最新版本\n"
            "• 所有必需的DLL文件是否在PATH中");
        return -1;  // 优雅地退出，不要拖泥带水
    }
    
    // 加载用户设置 - 记住用户的习惯，就像老朋友一样贴心
    QSettings settings;
    if (settings.contains("geometry")) {
        // 恢复窗口大小和位置 - 让用户感觉"回到家了"
        mainWindow.restoreGeometry(settings.value("geometry").toByteArray());
    }
    if (settings.contains("windowState")) {
        // 恢复窗口状态（最大化、停靠面板位置等）- 一切都是熟悉的样子
        mainWindow.restoreState(settings.value("windowState").toByteArray());
    }
    
    // 应用主题 - 让界面符合用户的审美偏好
    QString theme = settings.value("theme", "light").toString();  // 默认浅色主题，护眼第一
    mainWindow.SetTheme(theme);
    
    // 显示主窗口 - 隆重登场！
    mainWindow.show();
    
    // 关闭启动画面（如果存在的话）
    if (splash) {
        splash->finish(&mainWindow);  // 优雅地告别启动画面
        delete splash;               // 记得清理内存，好习惯要保持
    }
    
    // 显示欢迎消息（可选功能）- 给新用户一个热情的拥抱
    /*
    // TODO: 设计一个更友好的新手引导系统
    QTimer::singleShot(1000, [&mainWindow]() {
        QMessageBox::information(&mainWindow, "欢迎使用 Ander CAD", 
            "欢迎来到 Ander CAD v1.0.0！🎉\n\n"
            "这是一个受 Fusion 360 启发的CAD应用程序，\n"
            "基于 Qt5 和 OpenCASCADE 构建。\n\n"
            "主要功能：\n"
            "• 参数化3D建模\n"
            "• 约束草图绘制\n"
            "• STEP/IGES/STL 导入导出\n"
            "• 交互式3D查看器\n"
            "• 现代化深色/浅色主题\n\n"
            "按F1获取帮助，或查看帮助菜单了解更多信息。");
    });
    */
    
    // 运行应用程序 - 把控制权交给Qt的事件循环
    // 从这里开始，程序就"活"起来了，开始响应用户的操作
    int result = app.exec();
    
    // 退出前保存设置 - 记住这次的状态，下次启动时恢复
    // 就像合上书时夹个书签，下次翻开还能接着读
    QSettings saveSettings;
    saveSettings.setValue("geometry", mainWindow.saveGeometry());      // 保存窗口大小位置
    saveSettings.setValue("windowState", mainWindow.saveState());      // 保存窗口状态
    
    // 返回程序退出码 - 告诉操作系统我们是正常退出还是出了什么问题
    return result;
}