#! /bin/bash

mkdir -p /etc/systemd/system
mv what-bot-arm64.service /etc/systemd/system
chmod 664 /etc/systemd/system/what-bot-arm64.service
mkdir -p /usr/local/bin/what-bot
mv bot-linux-arm64 /usr/local/bin/what-bot/what-bot-arm64
mv token.txt /usr/local/bin/what-bot/
chmod +x /usr/local/bin/what-bot/what-bot-arm64
systemctl daemon-reload
systemctl enable what-bot-arm64
systemctl start what-bot-arm64
systemctl status what-bot-arm64
