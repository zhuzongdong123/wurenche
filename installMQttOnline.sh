#!/bin/bash
 
# 步骤1：更新系统并安装 Mosquitto
sudo apt update 
sudo apt install -y mosquitto mosquitto-clients 
 
# 步骤2：配置自定义端口和密码 
echo -e "\n# 自定义配置" | sudo tee -a /etc/mosquitto/mosquitto.conf
echo "listener 15248" | sudo tee -a /etc/mosquitto/mosquitto.conf 
echo "allow_anonymous false" | sudo tee -a /etc/mosquitto/mosquitto.conf
echo "password_file /etc/mosquitto/passwd" | sudo tee -a /etc/mosquitto/mosquitto.conf 
 
# 步骤3：设置密码文件
sudo touch /etc/mosquitto/passwd
sudo mosquitto_passwd -b /etc/mosquitto/passwd root Chrtc@88
 
# 步骤4：设置权限并重启服务 
sudo chmod 600 /etc/mosquitto/passwd
sudo systemctl restart mosquitto
sudo systemctl enable mosquitto
 
# 步骤5：开放防火墙端口 
sudo ufw allow 15248/tcp
sudo ufw reload
 
# 步骤6：验证服务状态
systemctl status mosquitto --no-pager
