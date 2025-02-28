#! /bin/bash

mkdir -p /etc/systemd/system
mv what-bot-x64.service /etc/systemd/system
chmod 664 /etc/systemd/system/what-bot-x64.service
mkdir -p /usr/local/bin/what-bot
mv bot-linux-x64 /usr/local/bin/what-bot/what-bot-x64
mv token.env /usr/local/bin/what-bot/
chmod +x /usr/local/bin/what-bot/what-bot-x64
systemctl daemon-reload
systemctl enable what-bot-x64
systemctl start what-bot-x64
systemctl status what-bot-x64
