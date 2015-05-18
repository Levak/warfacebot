#!/usr/bin/env bash
# Usage: wb.sh [eu|na|tr]

read -p "Email: " email
read -s -p "Password: " psswd

echo
echo -n 'Connecting...'

res=$(curl -ks --data "email=${email}&pwd=${psswd}" \
     'https://gface.com/api/-text/auth/login.json')

echo "$res" | grep 'fail' && exit 1
echo 'done'

token=$(echo "$res" | sed 's/^.*token":"\([-0-9a-f]*\).*$/\1/')
userid=$(echo "$res" | sed 's/^.*userid":\([0-9]*\).*$/\1/')

./wb ${token} ${userid} $1
