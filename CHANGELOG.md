Rework message handling system.

Rework so when looking data up it reply the last message not sent by the user.
It also ignores messages sent by any bot.
It added mutex to slow down the speed of the bot.
It dropped all arm support.