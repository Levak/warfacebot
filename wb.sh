#!/usr/bin/env bash

WB=${WB:-"./wb"}
WBD=${WBD:-"./wbd"}

usage()
{
    echo "Usage: wb.sh eu|na|tr|br|vn [OPTIONS]"
    exit 1
}

error()
{
    echo "failed"
    exit $1
}

[ $# -lt 1 ] && usage
cd `dirname $0`

if [ -z ${login} ]; then
    read -p "Login: " login
fi
if [ -z ${psswd} ]; then
    read -s -p "Password: " psswd
fi

echo
echo -n 'Connecting...'

server="./cfg/server/${1}.cfg"

case "$1" in
    eu|na|tr )
        email="${login}"
        res=$(curl -ks -X POST \
            --data-urlencode "email=${email}" \
            --data-urlencode "password=${psswd}" \
            'https://launcher.warface.com/app/auth') || error 3

        if echo "$res" | grep code >/dev/null; then
            if echo "$res" | grep 10020 >/dev/null; then
                eula=$(echo "$res" | sed 's/^.*eulaversion":\([0-9]*\).*$/\1/')
                res=$(curl -c- -Lks -X POST \
                    -H "Host: www.warface.com" \
                    -H "X-Requested-With: XMLHttpRequest" \
                    --data-urlencode "email=${email}" \
                    --data-urlencode "password=${psswd}" \
                    --data-urlencode "eulaversion=${eula}" \
                    'https://www.warface.com/en/session/login') || error 3

                res=$(curl -ks -X POST \
                    --data-urlencode "email=${email}" \
                    --data-urlencode "password=${psswd}" \
                    'https://launcher.warface.com/app/auth') || error 3
            else
                echo "$res"
                error 1
            fi
        fi

        echo "$res" | grep 'code' && error 1

        token=$(echo "$res" | sed 's/^.*sessionToken":"\([-0-9a-f]*\).*$/\1/')

        res=$(curl -ks -G \
            --data-urlencode "token=${token}" \
            'https://rest.api.gface.com/gface-rest/user/get/my.json') || error 3

        echo "$res" | grep 'code' && error 1

        userid=$(echo "$res" | sed 's/^.*owner":[^"]*"id":\([0-9]*\).*$/\1/')

        echo 'done'
        ;;

    vn )
        username="${login}"
        ua='libcurl-agent/1.0'
        cpid=1000
        client_id="AAER47Ux4Yb1BCeoPGxODVEjGq25cKwOOklTHEIE"
        client_secret="7YzKxfLpp3HYnyQY0HMeRXE8ijIblNsJ5adnABe3O0iHvAdnAClQRXs3vcAoMu"
        redirect_uri="http://goplay.vn"
        CID="?client_id=${client_id}&client_secret=${client_secret}"

        psswd=$(echo -n "$psswd" | md5sum | sed 's/ .*//')

        ip=$(curl -s \
            -A "$ua" \
            'http://share.goplay.vn/Launcherservice/checkip.aspx')

        res=$(curl -s \
            -A "$ua" \
            --data-urlencode "username=${username}" \
            'http://billing.graph.go.vn/authentication/salt'"${CID}") || error 3

        echo "$res" | grep -- '"_code":-' && error 1

        salt=$(echo "$res" | sed 's/^.*_data":"\([^"]*\).*$/\1/')

        password=$(echo -n "${username}${psswd}${salt}"| md5sum | sed 's/ .*//')

        res=$(curl -s \
            -A "$ua" \
            --data-urlencode "username=${username}" \
            --data-urlencode "password=${password}" \
            --data-urlencode "cpId=${cpid}" \
            --data-urlencode "ip=${ip}" \
            'http://billing.graph.go.vn/authentication/login'"${CID}") || error 3

        echo "$res" | grep -- '"_code":-' && error 1

        code=$(echo "$res" | sed 's/^.*code":"\([^"]*\).*$/\1/')
        userid=$(echo "$res" | sed 's/^.*UserId":\([0-9]*\).*$/\1/')

        res=$(curl -s \
            -A "$ua" \
            --data-urlencode "client_id=${client_id}" \
            --data-urlencode "client_secret=${client_secret}" \
            --data-urlencode "code=${code}" \
            --data-urlencode "redirect_uri=${redirect_uri}" \
            'http://billing.graph.go.vn/oauth/access_token'"${CID}") || error 3

        echo "$res" | grep -- '"_code":-' && error 1
        echo 'done'

        token=$(echo "$res" | sed 's/^.*access_token":"\([^"]*\).*$/\1/')

        ;;

    ru )
        echo "TODO"
        usage
        ;;

    br )
        username="${login}"
        res=$(curl -Gs \
            --data-urlencode "username=${username}" \
            'http://auth.warface.levelupgames.com.br/AuthenticationService.svc/GetSalt?') || error 3

        echo "$res" | grep 'false' && error 1

        salt=$(echo "$res" | sed 's/^.*<Salt>\([-0-9A-Z]*\).*$/\1/')

        psswd=$(echo -n "$psswd" | md5sum | awk '{print toupper($1)}')
        psswd=$(echo -n "$psswd""$salt" | sha1sum | awk '{print toupper($1)}')

        res1=$(curl -Gs \
            --data-urlencode "username=${username}" \
            --data-urlencode "password=${psswd}" \
            --data "ip=" \
            'http://auth.warface.levelupgames.com.br/AuthenticationService.svc/CreateToken?') || error 3

        echo "$res1" | grep 'excedido' && error 1
        echo "$res1" | grep 'false' && error 1

        echo 'done'

        token=$(echo "$res1" | sed 's/^.*<Token>\([-0-9a-f]*\).*$/\1/')

        echo
        echo -n Token Generated is $token

        userid=$username
        echo
        ;;

    * )
        echo "Unimplemented"
        usage
        ;;
esac

if [ -z $WB_AS_DAEMON ]; then
    ${WB} -t ${token} -i ${userid} -f ${server} $@
else
    if [ -z $NO_FORK ]; then
        ${WBD} -t ${token} -i ${userid} -f ${server} $@ &
    else
        ${WB} -t ${token} -i ${userid} -f ${server} $@
    fi
fi

