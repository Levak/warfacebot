#!/usr/bin/env bash
# Usage: wb.sh [eu|na|tr|vn]

cd `dirname $0`

error()
{
    echo "failed"
    exit $1
}

case "$1" in
    eu|na|tr )
        read -p "Email: " email
        read -s -p "Password: " psswd
        echo
        echo -n 'Connecting...'

        res=$(curl -ks \
            --data-urlencode "email=${email}" \
            --data-urlencode "pwd=${psswd}" \
            'https://gface.com/api/-text/auth/login.json') || error 3

        echo "$res" | grep 'fail' && error 1
        echo 'done'

        token=$(echo "$res" | sed 's/^.*token":"\([-0-9a-f]*\).*$/\1/')
        userid=$(echo "$res" | sed 's/^.*userid":\([0-9]*\).*$/\1/')
        ;;

    vn )
        read -p "Username: " username
        read -s -p "Password: " psswd
        echo
        echo -n 'Connecting...'

        psswd=$(echo -n "$psswd" | md5sum | sed 's/ .*//')
        ip=$(dig +short myip.opendns.com @resolver1.opendns.com)

        res=$(curl -Gs \
            --data-urlencode "username=${username}" \
            --data-urlencode "password=${psswd}" \
            --data "cpid=100001" \
            --data "clientip=${ip}" \
            --data "sign=9a8c2995cc35eb97ec33b203c68200c9" \
            'http://account.goplay.vn/fastlogin') || error 3

        echo "$res" | grep -- '"ret":-' && error 1
        echo 'done'

        token=$(echo "$res" | sed 's/^.*token":"\([^"]*\).*$/\1/')
        userid=$(echo "$res" | sed 's/^.*ret":\([0-9]*\).*$/\1/')
        ;;

    * )
        echo "Unimplemented"
        error 2
        ;;
esac

./wb ${token} ${userid} $1
