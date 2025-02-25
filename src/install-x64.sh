#! /bin/bash

mkdir -p ~/.config/systemd/user
mv what-bot-x64.service ~/.config/systemd/user/
chmod 664 ~/.config/systemd/user/what-bot-x64.service
mkdir -p /usr/local/bin/what-bot
mv bot-linux-x64 /usr/local/bin/what-bot/what-bot-x64
mv token.txt /usr/local/bin/what-bot/
chmod +x /usr/local/bin/what-bot/what-bot-x64
systemctl daemon-reload
systemctl --user enable what-bot-x64
systemctl --user start what-bot-x64
systemctl --user status what-bot-x64