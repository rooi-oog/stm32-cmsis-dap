#!/usr/bin/bash

stty -F $1 -opost -onlcr -isig -icanon -iexten -echo -echoe -echok -echoctl -echoke ospeed 4000000 ispeed 4000000