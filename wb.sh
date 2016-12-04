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
		
		for i in {1..5}; do  
			res=$(curl -Lks -X POST \
				-H "Host: source-lw-eu.warface.com" \
				-H "X-User-Agent: u-launcher GFL" \
				--data-urlencode "email=${email}" \
				--data-urlencode "password=${psswd}" \
				--data-urlencode "g-recaptcha-response=0" \
				'https://source-lw-eu.warface.com/en/session/login-com/') || error 3
			echo "$res" | grep 'Captcha' && echo 'Bad luck, trying again in 5sec...' && sleep 5 && continue
			break
		done
		
		echo "$res" | grep 'Captcha' && error 1
		
        token=$(echo "$res" \
            | sed -r 's/^.*"token":"([^"]+)".*$/\1/')
			
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

        sleep 20
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

