#include "paramsdao.h"
#include <QUuid>
#include <QDebug>
#include <QJsonArray>
#include "appcommonbase.h"
#include "appeventbase.h"
#include <QDateTime>

ParamsDao::ParamsDao()
{

}

void ParamsDao::createAllTable()
{
    //创建表(参数信息表) ========验证通过
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id" << "road_name" << "pile_num" << "up_and_down" << "technical_level" << "lane" << "opt_person" << "opt_unit" << "save_path" ;
        sType << "varchar(32)" << "varchar(128)" << "varchar(20)" << "varchar(8)" << "varchar(20)" << "varchar(128)" << "varchar(20)" << "varchar(128)" << "varchar(512)";
        bool result = m_sqlite.initTable("params",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") +  "参数表创建失败");
        }
    }

    //创建表(位置信息表GPFPD) ========验证通过
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id" << "type" << "GPSWeek" << "GPSTime" << "heading" << "pitch" << "roll"
                  << "longitude" << "altitude" << "lattitude"
                  << "ve" << "vn" << "vu" << "speed" << "baseLine" << "NSV1" << "NSV2" << "Status" << "Cs"
                  << "pulse_value" << "create_time" << "GPS_time";
        sType << "integer PRIMARY KEY autoincrement" << "int(4)" << "double(8)" << "double(8)" << "float(4)" << "float(4)" << "float(4)"
              << "double(8)" << "double(8)" << "float(4)"
              << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)"
              << "int(8)" << "datetime(8)" << "datetime(8)";
        bool result = m_sqlite.initTable("location_GPFPDDMI",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "位置表(GPFPD)创建失败");
        }
    }

    //创建表(位置信息表GPGGA) ========验证通过
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id" << "utc" << "Latitude" << "N"
                  << "Longitude" << "E" << "FS"
                  << "NoSV" << "HDOP" << "msl" << "msl_unit" << "Altref" << "Altref_unit" << "DiffAge" << "DiffStation"
                   << "pulse_value" << "create_time" << "GPS_time";
        sType << "integer PRIMARY KEY autoincrement" << "varchar(20)" << "varchar(20)" << "varchar(20)"
              << "varchar(20)" << "varchar(20)" << "varchar(20)"
              << "varchar(20)" << "varchar(20)" << "varchar(20)"<< "varchar(20)" << "varchar(20)" << "varchar(20)"<< "varchar(20)" << "varchar(20)"
               << "varchar(64)" << "varchar(24)" << "varchar(24)";
        bool result = m_sqlite.initTable("location_GPGGA",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "位置表(GPGGA)创建失败");
        }
    }

    //创建表(位置信息表GPRMC) ========验证通过
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id" << "utc" << "status" << "latitude"
                  << "N" << "longitude" << "E"
                  << "Spd" << "cog" << "ddmmyy" << "mv" << "mvE" << "mode"
                   << "pulse_value" << "create_time" << "GPS_time";
        sType << "integer PRIMARY KEY autoincrement" << "varchar(20)" << "varchar(20)" << "varchar(20)"
              << "varchar(20)" << "varchar(20)" << "varchar(0)"
              << "varchar(20)" << "varchar(20)" << "varchar(20)"<< "varchar(20)" << "varchar(20)" << "varchar(20)"
               << "varchar(64)" << "varchar(24)" << "varchar(24)";
        bool result = m_sqlite.initTable("location_GPRMC",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "位置表(GPRMC)创建失败");
        }
    }

    //创建表(位置信息表GPPOSDMI) ========验证通过
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id" << "type" << "GPSWeek" << "GPSTime" << "heading" << "pitch" << "roll"
                  << "lattitude" << "longitude"  << "altitude"
                  << "GyroX" << "GyroY" << "GyroZ"
                  << "AccX" << "AccY" << "AccZ"
                  << "ve" << "vn" << "vu" << "speed" << "baseLine" << "NSV1" << "NSV2" << "Status" << "Cs"
                  << "pulse_value" << "create_time" << "GPS_time";
        sType << "integer PRIMARY KEY autoincrement" << "int(4)" << "double(8)" << "double(8)" << "float(4)" << "float(4)" << "float(4)"
              << "double(8)" << "double(8)" << "float(4)"
              << "varchar(20)" << "varchar(20)" << "varchar(20)"
              << "varchar(20)" << "varchar(20)" << "varchar(20)"
              << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)" << "float(4)"
              << "int(8)" << "datetime(8)" << "datetime(8)";

        bool result = m_sqlite.initTable("location_GPPOSDMI",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "位置表(GPPOSDMI)创建失败");
        }
    }

    //创建表(位置标注表) ========验证通过event_marker
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id" << "type" << "marker_text"  << "longitude" << "lattitude" << "pulse_value" << "current_distance" << "current_station" << "create_time" << "GPS_time" ;
        sType << "integer PRIMARY KEY autoincrement" << "int(8)" << "varchar(128)"  << "double(8)" << "double(8)" << "int(8)" << "double(8)" << "double(8)" << "datetime(8)" << "datetime(8)";
        bool result = m_sqlite.initTable("event_marker",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "事件标注表创建失败");
        }
    }

    //创建表(校桩表) ========验证通过station_info
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id"  << "longitude" << "lattitude"  << "pulse_value" << "current_distance" << "correct_station" << "create_time" << "GPS_time" ;
        sType << "integer PRIMARY KEY autoincrement"  << "varchar(20)" << "double(8)" << "double(8)" << "int(8)" << "double(8)" << "double(8)" << "datetime(8)" << "datetime(8)";
        bool result = m_sqlite.initTable("station_info",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "校桩表创建失败");
        }
    }

    //创建表(图片表)面阵相机
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "path" << "longitude" << "lattitude" << "heading" << "pulse_value" << "create_time" << "id";
        sType << "varchar(36)" << "double(8)" << "double(8)" << "double(8)" << "int(8)" << "datetime(8)" << "int(8)";
        bool result = m_sqlite.initTable("landscape_pic",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "图片表创建失败");
        }
    }

    //创建表(图片表) 左线阵相机
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "path"  << "id" << "create_time" << "pave_type";
        sType << "varchar(36)"  << "int(8)" << "datetime(8)" << "varchar(8)";
        bool result = m_sqlite.initTable("road_pic_L",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "图片表创建失败");
        }
    }

    //创建表(图片表) 右线阵相机
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "path" << "id" << "create_time" << "pave_type";
        sType << "varchar(36)"  << "int(8)" << "datetime(8)" << "varchar(8)";
        bool result = m_sqlite.initTable("road_pic_R",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "图片表创建失败");
        }
    }
    //创建表(加速度表)
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id"  << "x_value" << "y_value" << "z_value"  << "pulse_value" << "create_time" << "GPS_time";
        sType << "integer PRIMARY KEY autoincrement" << "double" << "double" << "double"  << "int(8)" << "datetime(8)" << "datetime(8)";
        bool result = m_sqlite.initTable("accelerometer",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "加速度表创建失败");
        }
    }

    //创建表(平整度仪表)
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id" << "start_num" << "end_num" << "iri"  << "pulse_value" << "create_time" << "GPS_time";
        sType << "integer PRIMARY KEY autoincrement" << "double" << "double" << "double"  << "int(8)" << "datetime(8)" << "datetime(8)";
        bool result = m_sqlite.initTable("DATA_IRI",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "平整度仪表创建失败");
        }
    }
    //创建表(平整度仪表)
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id"  << "distance" << "speed" << "longitude" << "lattitude"  << "pulse_value" << "create_time" << "GPS_time";
        sType << "integer PRIMARY KEY autoincrement" << "double" << "double" << "double" << "double"  << "int(8)" << "datetime(8)" << "datetime(8)";
        bool result = m_sqlite.initTable("IRI_location",sNameList,sType);
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "平整度仪表创建失败");
        }
    }

    //创建表(日志操作表)
    {
        QStringList sNameList;
        QStringList sType;
        sNameList << "id"  << "log_text" << "create_time";
        sType << "integer PRIMARY KEY autoincrement" << "text" << "datetime(8)";
        m_sqlite.initTable("LOG",sNameList,sType);
    }
}

bool ParamsDao::saveParams(QJsonObject &obj)
{
    if(obj.value("savePath").toString().isEmpty())
    {
        return false;
    }

    //创建数据库和表
    if(!m_sqlite.getDB().isOpen())
    {
        bool result = m_sqlite.createDB(obj.value("savePath").toString());
        if(!result)
        {
            emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "数据库创建失败");
        }
        createAllTable();
    }

    //清空表，保存参数数据只有一条
    m_sqlite.clearTable("params");

    //插入参数
    QMap<QString, QVariantList> mapData;
    QString uuid = QUuid::createUuid().toString().remove("{").remove("}").remove("-");
    obj.insert("id",uuid);
    mapData.insert("id",QVariantList() << uuid);
    mapData.insert("road_name",QVariantList() << obj.value("roadName").toString());
    mapData.insert("pile_num",QVariantList() << obj.value("pileNum").toString());
    mapData.insert("up_and_down",QVariantList() << obj.value("upAndDown").toString());
    mapData.insert("technical_level",QVariantList() << obj.value("technicalLevel").toString());
    mapData.insert("lane",QVariantList() << obj.value("lane").toString());
    mapData.insert("opt_person",QVariantList() << obj.value("optPerson").toString());
    mapData.insert("opt_unit",QVariantList() << obj.value("optUnit").toString());
    mapData.insert("save_path",QVariantList() << obj.value("savePath").toString());
    bool result = m_sqlite.insertDataForBase("params",mapData);
    if(result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "参数保存成功");
    }
    else
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "参数保存失败");
    }
    return result;
}

bool ParamsDao::saveLog(QJsonObject &obj)
{
    //插入参数
    QMap<QString, QVariantList> mapData;
    mapData.insert("log_text",QVariantList() << obj.value("text").toString());
    mapData.insert("create_time",QVariantList() << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
    return m_sqlite.insertDataForBase("LOG",mapData);
}

bool ParamsDao::saveGPSInfo_GPFPD(QJsonObject &obj)
{
    QVariantList id_list;
    QVariantList week_list;
    QVariantList time_list;
    QVariantList longitude_list;
    QVariantList lattitude_list;
    QVariantList altitude_list;
    QVariantList create_time_list;
    QVariantList pulse_value_list;
    QVariantList heading_list;
    QVariantList pitch_list;
    QVariantList roll_list;
    QVariantList ve_list;
    QVariantList vn_list;
    QVariantList vu_list;
    QVariantList speed_list;
    QVariantList baseLine_list;
    QVariantList NSV1_list;
    QVariantList NSV2_list;
    QVariantList status_list;
    QVariantList type_list;
    QVariantList cs_list;
    QVariantList gps_time_list;

    auto array =obj.value("data").toArray();
    if(0 == array.size())
    {
        return true;
    }

    for(auto dataObj : array)
    {
        longitude_list << dataObj.toObject().value("lon").toDouble();
        lattitude_list << dataObj.toObject().value("lat").toDouble();
        altitude_list << dataObj.toObject().value("altitude").toDouble();
        create_time_list << dataObj.toObject().value("createTime").toString();
        pulse_value_list << dataObj.toObject().value("pulse_value").toInt();
        heading_list << dataObj.toObject().value("heading").toDouble();
        pitch_list << dataObj.toObject().value("pitch").toDouble();
        roll_list << dataObj.toObject().value("roll").toDouble();
        ve_list << dataObj.toObject().value("ve").toDouble();
        vn_list << dataObj.toObject().value("vn").toDouble();
        vu_list << dataObj.toObject().value("vu").toDouble();
        type_list << dataObj.toObject().value("type").toInt();
        week_list<< dataObj.toObject().value("GPSWeek").toDouble();
        time_list<< dataObj.toObject().value("GPSTime").toDouble();
        speed_list<< dataObj.toObject().value("speed").toDouble();
        baseLine_list<< dataObj.toObject().value("baseLine").toDouble();
        NSV1_list<< dataObj.toObject().value("NSV1").toDouble();
        NSV2_list<< dataObj.toObject().value("NSV2").toDouble();
        status_list<< dataObj.toObject().value("Status").toString();
        cs_list<< dataObj.toObject().value("Cs").toString();
        gps_time_list<< dataObj.toObject().value("GPS_time").toString();
    }

    QVector<QVariantList> listVec;
    listVec.push_back(longitude_list);
    listVec.push_back(lattitude_list);
    listVec.push_back(altitude_list);
    listVec.push_back(create_time_list);
    listVec.push_back(pulse_value_list);
    listVec.push_back(heading_list);
    listVec.push_back(pitch_list);
    listVec.push_back(roll_list);
    listVec.push_back(ve_list);
    listVec.push_back(vn_list);
    listVec.push_back(vu_list);
    listVec.push_back(type_list);
    listVec.push_back(week_list);
    listVec.push_back(time_list);
    listVec.push_back(speed_list);
    listVec.push_back(baseLine_list);
    listVec.push_back(NSV1_list);
    listVec.push_back(NSV2_list);
    listVec.push_back(status_list);
    listVec.push_back(cs_list);
    listVec.push_back(gps_time_list);

    QStringList sNameList;
    sNameList << "longitude";
    sNameList << "lattitude";
    sNameList << "altitude";
    sNameList << "create_time";
    sNameList << "pulse_value";
    sNameList << "heading";
    sNameList << "pitch";
    sNameList << "roll";
    sNameList << "ve";
    sNameList << "vn";
    sNameList << "vu";
    sNameList << "type";
    sNameList << "GPSWeek";
    sNameList << "GPSTime";
    sNameList << "speed";
    sNameList << "baseLine";
    sNameList << "NSV1";
    sNameList << "NSV2";
    sNameList << "Status";
    sNameList << "Cs";
    sNameList << "GPS_time";
    bool result = m_sqlite.insertDatasForBase("location_GPFPDDMI",sNameList,listVec,true);
    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "位置信息(GPFPD)保存失败");
    }
    return result;
}

bool ParamsDao::saveGPSInfo_GPPOSDMI(QJsonObject &obj)
{
    QVariantList id_list;
    QVariantList week_list;
    QVariantList time_list;
    QVariantList longitude_list;
    QVariantList lattitude_list;
    QVariantList altitude_list;
    QVariantList create_time_list;
    QVariantList pulse_value_list;
    QVariantList heading_list;
    QVariantList pitch_list;
    QVariantList roll_list;
    QVariantList ve_list;
    QVariantList vn_list;
    QVariantList vu_list;
    QVariantList speed_list;
    QVariantList baseLine_list;
    QVariantList NSV1_list;
    QVariantList NSV2_list;
    QVariantList status_list;
    QVariantList type_list;
    QVariantList cs_list;
    QVariantList GyroX_list;
    QVariantList GyroY_list;
    QVariantList GyroZ_list;
    QVariantList AccX_list;
    QVariantList AccY_list;
    QVariantList AccZ_list;
    QVariantList gpsTime_list;

    auto array =obj.value("data").toArray();
    if(0 == array.size())
    {
        return true;
    }

    for(auto dataObj : array)
    {
        longitude_list << dataObj.toObject().value("lon").toDouble();
        lattitude_list << dataObj.toObject().value("lat").toDouble();
        altitude_list << dataObj.toObject().value("altitude").toDouble();
        create_time_list << dataObj.toObject().value("createTime").toString();
        pulse_value_list << dataObj.toObject().value("pulse_value").toInt();
        heading_list << dataObj.toObject().value("heading").toDouble();
        pitch_list << dataObj.toObject().value("pitch").toDouble();
        roll_list << dataObj.toObject().value("roll").toDouble();
        ve_list << dataObj.toObject().value("ve").toDouble();
        vn_list << dataObj.toObject().value("vn").toDouble();
        vu_list << dataObj.toObject().value("vu").toDouble();
        type_list << dataObj.toObject().value("type").toInt();
        week_list<< dataObj.toObject().value("GPSWeek").toDouble();
        time_list<< dataObj.toObject().value("GPSTime").toDouble();
        speed_list<< dataObj.toObject().value("speed").toDouble();
        baseLine_list<< dataObj.toObject().value("baseLine").toDouble();
        NSV1_list<< dataObj.toObject().value("NSV1").toDouble();
        NSV2_list<< dataObj.toObject().value("NSV2").toDouble();
        status_list<< dataObj.toObject().value("Status").toString();
        cs_list<< dataObj.toObject().value("Cs").toString();
        GyroX_list << dataObj.toObject().value("GyroX").toDouble();
        GyroY_list << dataObj.toObject().value("GyroY").toDouble();
        GyroZ_list << dataObj.toObject().value("GyroZ").toDouble();
        AccX_list << dataObj.toObject().value("AccX").toDouble();
        AccY_list << dataObj.toObject().value("AccY").toDouble();
        AccZ_list << dataObj.toObject().value("AccZ").toDouble();
        gpsTime_list << dataObj.toObject().value("GPS_time").toString();
    }

    QVector<QVariantList> listVec;
    listVec.push_back(longitude_list);
    listVec.push_back(lattitude_list);
    listVec.push_back(altitude_list);
    listVec.push_back(create_time_list);
    listVec.push_back(pulse_value_list);
    listVec.push_back(heading_list);
    listVec.push_back(pitch_list);
    listVec.push_back(roll_list);
    listVec.push_back(ve_list);
    listVec.push_back(vn_list);
    listVec.push_back(vu_list);
    listVec.push_back(type_list);
    listVec.push_back(week_list);
    listVec.push_back(time_list);
    listVec.push_back(speed_list);
    listVec.push_back(baseLine_list);
    listVec.push_back(NSV1_list);
    listVec.push_back(NSV2_list);
    listVec.push_back(status_list);
    listVec.push_back(cs_list);
    listVec.push_back(GyroX_list);
    listVec.push_back(GyroY_list);
    listVec.push_back(GyroZ_list);
    listVec.push_back(AccX_list);
    listVec.push_back(AccY_list);
    listVec.push_back(AccZ_list);
    listVec.push_back(gpsTime_list);

    QStringList sNameList;
    sNameList << "longitude";
    sNameList << "lattitude";
    sNameList << "altitude";
    sNameList << "create_time";
    sNameList << "pulse_value";
    sNameList << "heading";
    sNameList << "pitch";
    sNameList << "roll";
    sNameList << "ve";
    sNameList << "vn";
    sNameList << "vu";
    sNameList << "type";
    sNameList << "GPSWeek";
    sNameList << "GPSTime";
    sNameList << "speed";
    sNameList << "baseLine";
    sNameList << "NSV1";
    sNameList << "NSV2";
    sNameList << "Status";
    sNameList << "Cs";
    sNameList << "GyroX";
    sNameList << "GyroY";
    sNameList << "GyroZ";
    sNameList << "AccX";
    sNameList << "AccY";
    sNameList << "AccZ";
    sNameList << "GPS_time";
    bool result = m_sqlite.insertDatasForBase("location_GPPOSDMI",sNameList,listVec,true);
    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "位置信息(GPPOSDMI)保存失败");
    }
    return result;
}

bool ParamsDao::saveGPSInfo_GPGGA(QJsonObject &obj)
{
    QVariantList utc_list;
    QVariantList Latitude_list;
    QVariantList N_list;
    QVariantList Longitude_list;
    QVariantList E_list;
    QVariantList FS_list;
    QVariantList NoSV_list;
    QVariantList HDOP_list;
    QVariantList msl_list;
    QVariantList msl_unit_list;
    QVariantList Altref_list;
    QVariantList Altref_unit_list;
    QVariantList DiffAge_list;
    QVariantList DiffStation_list;
    QVariantList pulse_value_list;
    QVariantList createTime_list;
    QVariantList gps_time_list;

    auto array =obj.value("data").toArray();
    if(0 == array.size())
    {
        return true;
    }

    for(auto dataObj : array)
    {
        utc_list << dataObj.toObject().value("utc").toString();
        Latitude_list << dataObj.toObject().value("Latitude").toString();
        N_list << dataObj.toObject().value("N").toString();
        Longitude_list << dataObj.toObject().value("Longitude").toString();
        E_list << dataObj.toObject().value("E").toString();
        FS_list << dataObj.toObject().value("FS").toString();
        NoSV_list << dataObj.toObject().value("NoSV").toString();
        HDOP_list << dataObj.toObject().value("HDOP").toString();
        msl_list << dataObj.toObject().value("msl").toString();
        msl_unit_list << dataObj.toObject().value("msl_unit").toString();
        Altref_list << dataObj.toObject().value("Altref").toString();
        Altref_unit_list << dataObj.toObject().value("Altref_unit").toString();
        DiffAge_list << dataObj.toObject().value("DiffAge").toString();
        DiffStation_list << dataObj.toObject().value("DiffStation").toString();
        pulse_value_list << dataObj.toObject().value("pulse_value").toString();
        createTime_list << dataObj.toObject().value("createTime").toString();
        gps_time_list << dataObj.toObject().value("GPS_time").toString();
    }

    QVector<QVariantList> listVec;
    listVec.push_back(utc_list);
    listVec.push_back(Latitude_list);
    listVec.push_back(N_list);
    listVec.push_back(Longitude_list);
    listVec.push_back(E_list);
    listVec.push_back(FS_list);
    listVec.push_back(NoSV_list);
    listVec.push_back(HDOP_list);
    listVec.push_back(msl_list);
    listVec.push_back(msl_unit_list);
    listVec.push_back(Altref_list);
    listVec.push_back(Altref_unit_list);
    listVec.push_back(DiffAge_list);
    listVec.push_back(DiffStation_list);
    listVec.push_back(pulse_value_list);
    listVec.push_back(createTime_list);
    listVec.push_back(gps_time_list);

    QStringList sNameList;
    sNameList << "utc";
    sNameList << "Latitude";
    sNameList << "N";
    sNameList << "Longitude";
    sNameList << "E";
    sNameList << "FS";
    sNameList << "NoSV";
    sNameList << "HDOP";
    sNameList << "msl";
    sNameList << "msl_unit";
    sNameList << "Altref";
    sNameList << "Altref_unit";
    sNameList << "DiffAge";
    sNameList << "DiffStation";
    sNameList << "pulse_value";
    sNameList << "create_time";
    sNameList << "GPS_time";
    bool result = m_sqlite.insertDatasForBase("location_GPGGA",sNameList,listVec,true);
    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "位置信息(GPGGA)保存失败");
    }
    return result;
}

bool ParamsDao::saveGPSInfo_GPRMC(QJsonObject &obj)
{
    QVariantList utc_list;
    QVariantList status_list;
    QVariantList latitude_list;
    QVariantList N_list;
    QVariantList longitude_list;
    QVariantList E_list;
    QVariantList Spd_list;
    QVariantList cog_list;
    QVariantList ddmmyy_list;
    QVariantList mv_list;
    QVariantList mvE_list;
    QVariantList mode_list;
    QVariantList pulse_value_list;
    QVariantList createTime_list;
    QVariantList gps_time_list;

    auto array =obj.value("data").toArray();
    if(0 == array.size())
    {
        return true;
    }

    for(auto dataObj : array)
    {
        utc_list << dataObj.toObject().value("utc").toString();
        status_list << dataObj.toObject().value("status").toString();
        latitude_list << dataObj.toObject().value("latitude").toString();
        N_list << dataObj.toObject().value("N").toString();
        longitude_list << dataObj.toObject().value("longitude").toString();
        E_list << dataObj.toObject().value("E").toString();
        Spd_list << dataObj.toObject().value("Spd").toString();
        cog_list << dataObj.toObject().value("cog").toString();
        ddmmyy_list << dataObj.toObject().value("ddmmyy").toString();
        mv_list << dataObj.toObject().value("mv").toString();
        mvE_list << dataObj.toObject().value("mvE").toString();
        mode_list << dataObj.toObject().value("mode").toString();
        pulse_value_list << dataObj.toObject().value("pulse_value").toString();
        createTime_list << dataObj.toObject().value("createTime").toString();
        gps_time_list << dataObj.toObject().value("GPS_time").toString();
    }

    QVector<QVariantList> listVec;
    listVec.push_back(utc_list);
    listVec.push_back(status_list);
    listVec.push_back(latitude_list);
    listVec.push_back(N_list);
    listVec.push_back(latitude_list);
    listVec.push_back(E_list);
    listVec.push_back(Spd_list);
    listVec.push_back(cog_list);
    listVec.push_back(ddmmyy_list);
    listVec.push_back(mv_list);
    listVec.push_back(mvE_list);
    listVec.push_back(mode_list);
    listVec.push_back(pulse_value_list);
    listVec.push_back(createTime_list);
    listVec.push_back(gps_time_list);

    QStringList sNameList;
    sNameList << "utc";
    sNameList << "status";
    sNameList << "latitude";
    sNameList << "N";
    sNameList << "longitude";
    sNameList << "E";
    sNameList << "Spd";
    sNameList << "cog";
    sNameList << "ddmmyy";
    sNameList << "mv";
    sNameList << "mvE";
    sNameList << "mode";
    sNameList << "pulse_value";
    sNameList << "create_time";
    sNameList << "GPS_time";
    bool result = m_sqlite.insertDatasForBase("location_GPRMC",sNameList,listVec,true);
    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "位置信息(GPRMC)保存失败");
    }
    return result;
}

bool ParamsDao::searchGPSInfo_GPPOSDMI(QJsonObject &obj)
{
    {
         QSqlQuery queryResult;
         QString sql = "SELECT count(*) as count FROM location_GPPOSDMI WHERE type = '0'";
         bool result = m_sqlite.execSQL(queryResult,sql);
         if(result)
         {
             while (queryResult.next()) {
                 int count = queryResult.value("count").toInt();   // 通过别名获取结果
                 qDebug() << "GPPOSDMI中类型等于0的数据个数: " << QString::number(count);
             }
         }
     }
    {
         QSqlQuery queryResult;
         QString sql = "SELECT count(*) as count FROM location_GPPOSDMI WHERE type = '1'";
         bool result = m_sqlite.execSQL(queryResult,sql);
         if(result)
         {
             while (queryResult.next()) {
                 int count = queryResult.value("count").toInt();   // 通过别名获取结果
                 qDebug() << "GPPOSDMI中类型等于1的数据个数: " << QString::number(count);
             }
         }
     }
    {
         QSqlQuery queryResult;
         QString sql = "SELECT count(*) as count FROM location_GPPOSDMI WHERE type = '2'";
         bool result = m_sqlite.execSQL(queryResult,sql);
         if(result)
         {
             while (queryResult.next()) {
                 int count = queryResult.value("count").toInt();   // 通过别名获取结果
                 qDebug() << "GPPOSDMI中类型等于2的数据个数: " << QString::number(count);
             }
         }
     }
    return true;
}

bool ParamsDao::saveDataIRI(QJsonObject &obj)
{
    QVariantList start_num_list;
    QVariantList end_num_list;
    QVariantList iri_list;
    QVariantList pulse_value_list;
    QVariantList createTime_list;
    QVariantList gps_time_list;

    auto array =obj.value("data").toArray();
    if(0 == array.size())
    {
        return true;
    }

    for(auto dataObj : array)
    {
        start_num_list << dataObj.toObject().value("start_num").toDouble();
        end_num_list << dataObj.toObject().value("end_num").toDouble();
        iri_list << dataObj.toObject().value("iri").toDouble();
        pulse_value_list << dataObj.toObject().value("pulse_value").toInt();
        createTime_list << dataObj.toObject().value("createTime").toString();
        gps_time_list << dataObj.toObject().value("GPS_time").toString();
    }

    QVector<QVariantList> listVec;
    listVec.push_back(start_num_list);
    listVec.push_back(end_num_list);
    listVec.push_back(iri_list);
    listVec.push_back(pulse_value_list);
    listVec.push_back(createTime_list);
    listVec.push_back(gps_time_list);

    QStringList sNameList;
    sNameList << "start_num";
    sNameList << "end_num";
    sNameList << "iri";
    sNameList << "pulse_value";
    sNameList << "create_time";
    sNameList << "GPS_time";
    bool result = m_sqlite.insertDatasForBase("DATA_IRI",sNameList,listVec,true);
    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "IRI值保存失败");
    }
    return result;
}

bool ParamsDao::saveDataLocation(QJsonObject &obj)
{
    QVariantList distance_list;
    QVariantList speed_list;
    QVariantList lon_list;
    QVariantList lat_list;
    QVariantList pulse_value_list;
    QVariantList createTime_list;
    QVariantList gps_time_list;

    auto array =obj.value("data").toArray();
    if(0 == array.size())
    {
        return true;
    }

    for(auto dataObj : array)
    {
        distance_list << dataObj.toObject().value("distance").toDouble();
        speed_list << dataObj.toObject().value("speed").toDouble();
        lon_list << dataObj.toObject().value("longitude").toDouble();
        lat_list << dataObj.toObject().value("lattitude").toDouble();
        pulse_value_list << dataObj.toObject().value("pulse_value").toInt();
        createTime_list << dataObj.toObject().value("createTime").toString();
        gps_time_list << dataObj.toObject().value("GPS_time").toString();
    }

    QVector<QVariantList> listVec;
    listVec.push_back(distance_list);
    listVec.push_back(speed_list);
    listVec.push_back(lon_list);
    listVec.push_back(lat_list);
    listVec.push_back(pulse_value_list);
    listVec.push_back(createTime_list);
    listVec.push_back(gps_time_list);

    QStringList sNameList;
    sNameList << "distance";
    sNameList << "speed";
    sNameList << "longitude";
    sNameList << "lattitude";
    sNameList << "pulse_value";
    sNameList << "create_time";
    sNameList << "GPS_time";
    bool result = m_sqlite.insertDatasForBase("IRI_location",sNameList,listVec,true);
    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "GNSS值保存失败");
    }
    return result;
}

bool ParamsDao::saveEventMarker(QJsonObject &obj)
{
    //插入参数
    QMap<QString, QVariantList> mapData;
    mapData.insert("type",QVariantList() << obj.value("type").toString());
    mapData.insert("marker_text",QVariantList() << obj.value("marker").toString());
    mapData.insert("longitude",QVariantList() << obj.value("lon").toDouble());
    mapData.insert("lattitude",QVariantList() << obj.value("lat").toDouble());
    mapData.insert("create_time",QVariantList() << obj.value("createTime").toString());
    mapData.insert("GPS_time",QVariantList() << obj.value("GPS_time").toString());
    mapData.insert("pulse_value",QVariantList() << obj.value("pulse_value").toInt());
    mapData.insert("current_distance",QVariantList() << obj.value("current_distance").toDouble());
    mapData.insert("current_station",QVariantList() << obj.value("current_station").toDouble());
    bool result = m_sqlite.insertDataForBase("event_marker",mapData);
    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "事件标注操作失败");
    }
    else
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "事件标注操作成功");
    }
    return result;
}

bool ParamsDao::savePileNum(QJsonObject &obj)
{
    //插入参数
    QMap<QString, QVariantList> mapData;
    mapData.insert("correct_station",QVariantList() << obj.value("pileNum").toString());
    mapData.insert("longitude",QVariantList() << obj.value("lon").toDouble());
    mapData.insert("lattitude",QVariantList() << obj.value("lat").toDouble());
    mapData.insert("create_time",QVariantList() << obj.value("createTime").toString());
    mapData.insert("GPS_time",QVariantList() << obj.value("GPS_time").toString());
    mapData.insert("pulse_value",QVariantList() << obj.value("pulse_value").toInt());
    mapData.insert("current_distance",QVariantList() << obj.value("current_distance").toDouble());
    bool result = m_sqlite.insertDataForBase("station_info",mapData);
    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "校桩操作失败");
    }
    else
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "校桩操作成功");
    }
    return result;
}

bool ParamsDao::saveAccelerometer(QJsonObject &obj)
{
    QVariantList xVlaue_list;
    QVariantList yVlaue_list;
    QVariantList zVlaue_list;
    QVariantList pulse_value_list;
    QVariantList createTime_list;
    QVariantList gps_time_list;

    auto array =obj.value("data").toArray();
    if(0 == array.size())
    {
        return true;
    }

    //qDebug() << "savedata"<< array;
    for(auto dataObj : array)
    {
        double xValue = dataObj.toObject().value("xValue").toDouble();
        xVlaue_list << xValue;
        double yValue = dataObj.toObject().value("yValue").toDouble();
        yVlaue_list << yValue;
        double zValue = dataObj.toObject().value("zValue").toDouble();
        zVlaue_list << zValue;
        pulse_value_list << dataObj.toObject().value("pulse_value").toInt();
        createTime_list << dataObj.toObject().value("createTime").toString();
        gps_time_list << dataObj.toObject().value("GPS_time").toString();
    }

    //qDebug() << "savedata1"<< xVlaue_list;

    QVector<QVariantList> listVec;
    listVec.push_back(xVlaue_list);
    listVec.push_back(yVlaue_list);
    listVec.push_back(zVlaue_list);
    listVec.push_back(pulse_value_list);
    listVec.push_back(createTime_list);
    listVec.push_back(gps_time_list);

    QStringList sNameList;
    sNameList << "x_value";
    sNameList << "y_value";
    sNameList << "z_value";
    sNameList << "pulse_value";
    sNameList << "create_time";
    sNameList << "GPS_time";
    bool result = m_sqlite.insertDatasForBase("accelerometer",sNameList,listVec,true);
    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "加速度值保存失败");
    }
    return result;
}

bool ParamsDao::saveCameraInfo(QJsonObject &obj)
{
    QVariantList longitude_list;
    QVariantList lattitude_list;
    QVariantList path_list;
    QVariantList pulse_value_list;
    QVariantList save_path_list;
    QVariantList gps_time_list;
    QVariantList id_list;
    int type;
    QVariantList create_time_list;
    QVariantList heading_list;
    QVariantList pave_type_list;
    auto array =obj.value("data").toArray();
    if(0 == array.size())
    {
        return true;
    }

    for(auto dataObj : array)
    {
        //id_list <<  QUuid::createUuid().toString().remove("{").remove("}").remove("-");
        longitude_list << dataObj.toObject().value("lon").toDouble();
        lattitude_list << dataObj.toObject().value("lat").toDouble();
        path_list << dataObj.toObject().value("path").toString();
        type = dataObj.toObject().value("type").toInt();
        create_time_list << dataObj.toObject().value("createTime").toString();
        pulse_value_list << dataObj.toObject().value("pulse_value").toInt();
        save_path_list << dataObj.toObject().value("save_path").toString();
        gps_time_list << dataObj.toObject().value("GPS_time").toString();
        heading_list << dataObj.toObject().value("heading").toDouble();
        pave_type_list << dataObj.toObject().value("pave_type").toString();
        id_list << dataObj.toObject().value("rcv_count").toInt();
    }

    QVector<QVariantList> listVec;
    QStringList sNameList;
    bool result = false;
    //道路相机
    if(0 == type || 2 == type)
    {
        listVec.push_back(path_list);
        listVec.push_back(create_time_list);
        listVec.push_back(pulse_value_list);
        listVec.push_back(pave_type_list);
        sNameList << "path";
        sNameList << "create_time";
        sNameList << "id";
        sNameList << "pave_type";

        QString tableName = "road_pic_L";
        if(2 == type)
            tableName = "road_pic_R";

        result = m_sqlite.insertDatasForBase(tableName,sNameList,listVec,true);
    }
    //景观相机
    else
    {
        listVec.push_back(longitude_list);
        listVec.push_back(lattitude_list);
        listVec.push_back(path_list);
        listVec.push_back(create_time_list);
        listVec.push_back(pulse_value_list);
        listVec.push_back(heading_list);
        listVec.push_back(id_list);

        sNameList << "longitude";
        sNameList << "lattitude";
        sNameList << "path";
        sNameList << "create_time";
        sNameList << "pulse_value";
        sNameList << "heading";
        sNameList << "id";
        result = m_sqlite.insertDatasForBase("landscape_pic",sNameList,listVec,true);
    }


    if(!result)
    {
        emit AppEventBase::getInstance()->sig_sendServerMsg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ") + "相机图片信息保存失败");
    }
    return result;
}

void ParamsDao::slt_rcvData(QString url, QJsonObject obj)
{
    bool result = false;
    QList<QString> urlList = url.split('/');
    //保存GPS数据
    if(SAVE_GPS_INFO_GPFPD == url)
    {
        result = saveGPSInfo_GPFPD(obj);
    }
    //保存GPS数据
    else if(SAVE_GPS_INFO_GPPOSTMI == url)
    {
        result = saveGPSInfo_GPPOSDMI(obj);
    }
    //保存GPS数据
    else if(SAVE_GPS_INFO_GPGGA == url)
    {
        result = saveGPSInfo_GPGGA(obj);
    }
    //保存GPS数据
    else if(SAVE_GPS_INFO_GPRMC == url)
    {
        result = saveGPSInfo_GPRMC(obj);
    }
    //保存加速度数据
    else if(SAVE_ACCELEROMETER == url)
    {
        result = saveAccelerometer(obj);
    }
    //保存事件标注数据
    else if(SAVE_EVENT_MARK == url)
    {
        result = saveEventMarker(obj);
    }
    //保存校桩数据
    else if(UPDATE_PILENUM == url)
    {
        result = savePileNum(obj);
    }
    //保存相机信息
    else if(SAVE_CAMERA_INFO == url)
    {
        result = saveCameraInfo(obj);
    }
    //保存参数
    else if(SAVE_PARAMS == url)
    {
        result = saveParams(obj);
    }
    //保存平整度仪
    else if(SAVE_DATALOCAITON == url)
    {
        result = saveDataLocation(obj);
    }
    //保存平整度仪
    else if(SAVE_DATAIRI == url)
    {
        result = saveDataIRI(obj);
    }
    //保存日志
    else if(SAVE_LOG == url)
    {
        result = saveLog(obj);
    }
    //查询数据
    else if(SLECT_GPPOSTMI == url)
    {
        result = searchGPSInfo_GPPOSDMI(obj);
    }
}
