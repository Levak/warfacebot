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
    tr )
        if [ -z "$token" ]; then
            email="${login}"
            res=$(curl -Lks -X GET \
                --cookie "realm=turkey" \
                -H "Host: www.warface.com.tr" \
                'https://www.warface.com.tr/en/login') || error 3

            csrf=$(echo "$res" \
                | grep 'name="csrf-token"' \
                | sed 's/^.*value="\([0-9a-zA-Z_]*\)".*$/\1/')

            res=$(curl -D- -Lks -X POST \
                --cookie "realm=turkey" \
                -H "Host: www.warface.com.tr" \
                -H "X-Requested-With: XMLHttpRequest" \
                --data-urlencode "email=${email}" \
                --data-urlencode "password=${psswd}" \
                --data-urlencode "eulaversion=" \
                --data-urlencode "csrf-token=${csrf}" \
                'https://www.warface.com.tr/en/session/login') || error 3

            headers=$(echo "$res" | sed "/^\s*\r*$/q")
            body=$(echo "$res" | sed "1,/^\s*\r*$/d")

            echo "$body" | grep 'code' && error 1

            token=$(echo "$headers" \
                | grep 'sessionToken' \
                | sed 's/^.*sessionToken=\([-0-9a-f]*\);.*$/\1/')
        fi

        res=$(curl -ks -G \
            --data-urlencode "token=${token}" \
            'https://rest.api.gface.com/gface-rest/user/get/my.json') || error 3

        echo "$res" | grep 'code' && error 1

        userid=$(echo "$res" | sed 's/^.*owner":[^"]*"id":\([0-9]*\).*$/\1/')

        echo 'done'
        ;;

    eu|na)
        ProjectId=2000076
        ChannelId=35
        case "$1" in
            eu) ShardId=1;;
            na) ShardId=2;;
        esac

        res=$(curl -s \
            -A "Downloader/1940" \
            -d '<?xml version="1.0" encoding="UTF-8"?><Auth Username="'"${login}"'" Password="'"${psswd}"'" ChannelId="'"${ChannelId}"'"/>' \
            'https://authdl.my.com/mygc.php?hint=Auth')

        echo "$res" | grep -- 'ErrorCode' && error 1

        SessionKey=$(echo "$res" | sed 's/^.* SessionKey="\([^"]*\).*$/\1/')

        res=$(curl -s \
            -A "Downloader/1940" \
            -d '<?xml version="1.0" encoding="UTF-8"?><Login SessionKey="'"${SessionKey}"'" ProjectId="'"${ProjectId}"'" ShardId="'"${ShardId}"'"/>' \
            'https://authdl.my.com/mygc.php?hint=Login')

        echo "$res" | grep -- 'ErrorCode' && error 1

        userid=$(echo "$res" | sed 's/^.* GameAccount="\([^"]*\).*$/\1/')
        token=$(echo "$res" | sed 's/^.* Code="\([^"]*\).*$/\1/')

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

        sleep 2
        echo 'done'

        token=$(echo "$res" | sed 's/^.*access_token":"\([^"]*\).*$/\1/')

        ;;

    ru-* )
        ProjectId=1177
        ShardId=0
        SubProjectId=0

        res=$(curl -D- -s \
            --data-urlencode "Login=${login}" \
            --data-urlencode "Password=${psswd}" \
            --data-urlencode "Domain=mail.ru" \
            'https://auth.mail.ru/cgi-bin/auth') || error 3

        location=$(echo "$res" | grep 'Location' | sed 's/^[^ ]* //')

        echo "$location" | grep -- '?fail=1' && error 1

        Mpop=$(echo "$res" | grep 'Set-Cookie: Mpop=' | sed 's/^[^=]*=\([^;]*\).*$/\1/')

        res=$(curl -s \
            -A "Downloader/11010" \
            -d '<?xml version="1.0" encoding="UTF-8"?><AutoLogin ProjectId="'${ProjectId}'" SubProjectId="'${SubProjectId}'" ShardId="'${ShardId}'" Mpop="'${Mpop}'"/>' \
            'https://authdl.mail.ru/sz.php?hint=AutoLogin')

        userid=$(echo "$res" | sed 's/^.*PersId="\([^"]*\)".*$/\1/')
        token=$(echo "$res" | sed 's/^.*Key="\([^"]*\)".*$/\1/')

        echo "done"
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

