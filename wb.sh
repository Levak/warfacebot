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
        res=$(curl -c cookies -Lks -X POST \
            -A 'u-launcher GFL' \
            -H "Host: www.warface.com.tr" \
            -H "Referer: https://gflauncher.gface.com/app" \
            -H "Origin: https://gflauncher.gface.com" \
            -H "X-Requested-With: XMLHttpRequest" \
            --data-urlencode "email=${login}" \
            --data-urlencode "password=${psswd}" \
            --data-urlencode "eulaaccept=" \
            --data-urlencode "realm=turkey" \
            --data-urlencode "rememberme=0" \
            --data-urlencode "g-recaptcha-response=" \
            'https://www.warface.com.tr/en/session/login-com/') || error 3

        if echo "$res" | grep code >/dev/null; then
            if echo "$res" | grep 10020 >/dev/null; then
                eula=$(echo "$res" | sed 's/^.*eulaversion":\([0-9]*\).*$/\1/')
                echo "New EULA version accepted: ${eula}"

                res=$(curl -b cookies -Lks -X POST \
                    -A 'u-launcher GFL' \
                    -H "Host: www.warface.com.tr" \
                    -H "Referer: https://gflauncher.gface.com/app" \
                    -H "Origin: https://gflauncher.gface.com" \
                    -H "X-Requested-With: XMLHttpRequest" \
                    --data-urlencode "email=${login}" \
                    --data-urlencode "password=${psswd}" \
                    --data-urlencode "eulaaccept=${eula}" \
                    --data-urlencode "realm=turkey" \
                    --data-urlencode "rememberme=0" \
                    --data-urlencode "g-recaptcha-response=" \
                    'https://www.warface.com.tr/en/session/login-com/') || error 3
            else
                echo "$res"
                error 1
            fi
        fi


        token=$(echo "$res" \
            | grep 'token' \
            | sed 's/^.*"token":"\([-0-9a-f]*\)".*$/\1/')

        userid=$(echo "$res" \
            | grep 'userid' \
            | sed 's/^.*"userid":\([-0-9]*\).*$/\1/')

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
        ua="Downloader/12900 MailRuGameCenter/1290"
        ProjectId=1177
        ChannelId=27
        ShardId=0
        SubProjectId=0

        res=$(curl -D- -s \
            -A "$ua" \
            --data-urlencode "Login=${login}" \
            --data-urlencode "Password=${psswd}" \
            --data-urlencode "Domain=mail.ru" \
            'https://auth.mail.ru/cgi-bin/auth') || error 3

        location=$(echo "$res" | grep 'Location' | sed 's/^[^ ]* //')

        echo "$location" | grep -- '?fail=1' && error 1

        Mpop=$(echo "$res" | grep 'Set-Cookie: Mpop=' | sed 's/^[^=]*=\([^;]*\).*$/\1/')

        if [ -z "$Mpop" ]; then
            res=$(curl -s \
                -A "$ua" \
                -d '<?xml version="1.0" encoding="UTF-8"?><GcAuth UserId="0" UserId2="0" Username="'${login}'" Password="'${psswd}'" ChannelId="'${ChannelId}'"/>' \
                'https://authdl.mail.ru/ec.php?hint=GcAuth') || error 3

            echo "$res" | grep 'ErrorCode' && error 1

            Gctok=$(echo "$res" | sed 's/^.*Token="\([^"]*\)".*$/\1/')

            res=$(curl -s \
                -A "$ua" \
                -d '<?xml version="1.0" encoding="UTF-8"?><AutoLogin ProjectId="'${ProjectId}'" SubProjectId="'${SubProjectId}'" ShardId="'${ShardId}'" GcToken="'${Gctok}'"/>' \
                'https://authdl.mail.ru/sz.php?hint=AutoLogin')
        else
            res=$(curl -s \
                -A "$ua" \
                -d '<?xml version="1.0" encoding="UTF-8"?><AutoLogin ProjectId="'${ProjectId}'" SubProjectId="'${SubProjectId}'" ShardId="'${ShardId}'" Mpop="'${Mpop}'"/>' \
                'https://authdl.mail.ru/sz.php?hint=AutoLogin')
        fi

        echo "$res" | grep 'SZError' && error 1

        userid=$(echo "$res" | sed 's/^.*PersId="\([^"]*\)".*$/\1/')
        token=$(echo "$res" | sed 's/^.*Key="\([^"]*\)".*$/\1/')

        sleep 2
        echo "done"
        ;;

    br )
        username="${login}"
        res=$(curl -Gs \
            --data-urlencode "username=${username}" \
            'https://minhaconta.levelupgames.com.br/AuthenticationService.svc/GetSalt?') || error 3

        echo "$res" | grep 'false' && error 1

        salt=$(echo "$res" | sed 's/^.*<Salt>\([-0-9A-Z]*\).*$/\1/')

        psswd=$(echo -n "$psswd" | md5sum | awk '{print toupper($1)}')
        psswd=$(echo -n "$psswd""$salt" | sha1sum | awk '{print toupper($1)}')

        res1=$(curl -Gs \
            --data-urlencode "username=${username}" \
            --data-urlencode "password=${psswd}" \
            --data "ip=" \
            'https://minhaconta.levelupgames.com.br/AuthenticationService.svc/CreateToken?') || error 3

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

