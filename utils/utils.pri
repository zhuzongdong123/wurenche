#引入各种提供能力的工具，和业务无关。所有的项目都可以使用，支持跨平台

#单例启动
include(qtsingleapplication/qtsingleapplication.pri)
#http
include(http/http.pri)
#socket(tcp的客户端和服务端)
include(socket/socket.pri)
#浏览器加载引擎
include(webview/webview.pri)
#串口通信功能
include(serialport/serialport.pri)
#数据库连接能力类
include(sqlite/sqlite.pri)
# chart能力类
include(chart/chart.pri)
# Qt封装类
include(baseclass/baseclass.pri)

INCLUDEPATH += \
    $$PWD \

