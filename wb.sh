#!/usr/bin/env bash

read -p "Email: " email
read -s -p "Password: " psswd

echo
echo -n 'Connecting...'

res=$(curl -s --data "email=${email}&pwd=${psswd}" \
     'https://gface.com/api/-text/auth/login.json')

grep 'fail' <<< $res && exit 1
echo 'done'

token=$(sed 's/^.*token":"\([-0-9a-f]*\).*$/\1/' <<< ${res})
userid=$(sed 's/^.*userid":\([0-9]*\).*$/\1/' <<< ${res})

./wb ${token} ${userid} eu # Europe server (others: na/tr)
