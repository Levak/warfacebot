#!/usr/bin/env bash

WB=${WB:-"./wb"}
WBD=${WBD:-"./wbd"}

usage()
{
    echo "Usage: wb.sh eu|na|ru-alpha|ru-bravo|ru-charlie|br|vn [OPTIONS]"
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

realm=$1

case "$realm" in
    eu|na|ru-*)
        ua='Downloader/15740'

        case "$realm" in
            ru-*) ProjectId=1177; ChannelId=35;;
            *) ProjectId=2000076; ChannelId=35;;
        esac

        case "$realm" in
            ru-alpha) ShardId=0;;
            ru-bravo) ShardId=1;;
            ru-charlie) ShardId=2;;
            ru-delta) ShardId=3;;
        esac


        case "$realm" in
            eu|pvp|pts) ShardId=1;;
            na) ShardId=2;;
        esac

        cookie=cookie-${realm}.txt
        rm -f $cookie

        case "$login" in
            *@mail.ru|*@inbox.ru|*@list.ru|*@bk.ru)
                # Get a state cookie
                res=$(curl -L -s \
                    -A "$ua" \
                    -b $cookie -c $cookie \
                    'https://auth-ac.my.games/social/mailru')

                state=$(echo "$res" | grep -m1 'state=' | sed 's/^.*state=\([^&]*\).*$/\1/')

                # Get a act cookie
                res=$(curl -LD- -s \
                    -A "$ua" \
                    -b $cookie -c $cookie \
                    'https://account.mail.ru')

                act=$(echo "$res" | grep -m1 'Set-Cookie: act=' | sed 's/^[^=]*=\([^;]*\).*$/\1/')

                # Mailru oauth
                res=$(curl -L -s \
                    -A "$ua" \
                    -b $cookie -c $cookie \
                    -H 'Referer: https://account.mail.ru/login?opener=o2' \
                    -H 'Origin: https://account.mail.ru' \
                    -H 'Content-Type: application/x-www-form-urlencoded' \
                    --data-urlencode "username=${login}" \
                    --data-urlencode "Login=${login}" \
                    --data-urlencode "Password=${psswd}" \
                    --data-urlencode "password=${psswd}" \
                    --data-urlencode "act_token=${act}" \
                    --data-urlencode "page=https://o2.mail.ru/xlogin?authid=kbjooyiv.dej&client_id=bbddb88d19b84a62aedd1ffbc71af201&force_us=1&from=o2&logo_target=_none&no_biz=1&redirect_uri=https%3A%2F%2Fauth-ac.my.games%2Fsocial%2Fmailru_callback%2F&remind_target=_self&response_type=code&scope=&signup_target=_self&state=${state}" \
                    --data-urlencode 'new_auth_form=1' \
                    --data-urlencode 'FromAccount=opener=o2&twoSteps=1' \
                    --data-urlencode 'lang=en_US' \
                    'https://auth.mail.ru/cgi-bin/auth')

                cat $cookie | grep '\ssdcs\s' > /dev/null || error 1
                cat $cookie | grep '\so2csrf\s' > /dev/null || error 1

                o2csrf=$(cat $cookie | grep '\so2csrf\s' | sed 's/.*\s\([^\s]*\)$/\1/')

                # Login in mygames using oauth cookies
                res=$(curl -L -s \
                    -A "$ua" \
                    -b $cookie -c $cookie \
                    --data-urlencode "Page=https://o2.mail.ru/login?client_id=bbddb88d19b84a62aedd1ffbc71af201&response_type=code&scope=&redirect_uri=https%3A%2F%2Fauth-ac.my.games%2Fsocial%2Fmailru_callback%2F&state=${state}&no_biz=1" \
                    --data-urlencode "FailPage=https://o2.mail.ru/login?client_id=bbddb88d19b84a62aedd1ffbc71af201&response_type=code&scope=&redirect_uri=https%3A%2F%2Fauth-ac.my.games%2Fsocial%2Fmailru_callback%2F&state=${state}&no_biz=1&fail=1" \
                    --data-urlencode "login=${login}" \
                    --data-urlencode "o2csrf=${o2csrf}" \
                    --data-urlencode "mode=" \
                    -H "Referer: https://o2.mail.ru/xlogin?client_id=bbddb88d19b84a62aedd1ffbc71af201&response_type=code&scope=&redirect_uri=https%3A%2F%2Fauth-ac.my.games%2Fsocial%2Fmailru_callback%2F&state=${state}&no_biz=1&force_us=1&signup_target=_self&remind_target=_self&logo_target=_none" \
                    -H 'Origin: https://o2.mail.ru' \
                    "https://o2.mail.ru/login")

                # Get a SDCS cookie
                res=$(curl -L -s \
                    -A "$ua" \
                    -b $cookie -c $cookie \
                    -H 'Referer: https://o2.mail.ru/xlogin' \
                    -H 'Origin: https://o2.mail.ru' \
                    'https://auth-ac.my.games/sdc?from=https%3A%2F%2Fapi.my.games%2Fsocial%2Fprofile%2Fsession&JSONP_call=callback1522169')
                ;;

            *)
                # Directly login to mygames
                res=$(curl -LD- -s \
                    -A "$ua" \
                    -b $cookie -c $cookie \
                    -H 'Referer: https://api.my.games/gamecenter/login/?lang=en_US' \
                    -H 'Origin: https://api.my.games' \
                    -H 'Content-Type: application/x-www-form-urlencoded' \
                    --data-urlencode "email=${login}" \
                    --data-urlencode "password=${psswd}" \
                    --data-urlencode 'continue=https://auth-ac.my.games/sdc?from=https%3A%2F%2Fapi.my.games%2Fgamecenter%2Flogin_finished%2F' \
                    --data-urlencode 'failure=https://api.my.games/gamecenter/login/' \
                    --data-urlencode 'nosavelogin=0' \
                    'https://auth-ac.my.games/auth' )

                echo "$res" | grep -- 'error_code=' && error 1
                ;;
        esac

        mc=$(cat $cookie | grep 'my.games' | grep '\smc\s' | sed 's/.*[[:space:]]\([^[:space:]]*\)$/\1/')
        sdcs=$(cat $cookie | grep 'my.games' | grep '\ssdcs\s' | sed 's/.*[[:space:]]\([^[:space:]]*\)$/\1/')

        [ -z "$sdcs" -o -z "$mc" ] && error 4




        res=$(curl -s \
            -A "$ua" \
            -d '<?xml version="1.0" encoding="UTF-8"?><Auth mc="'"${mc}"'" sdcs="'"${sdcs}"'" ChannelId="'"${ChannelId}"'" GcLang="en" UserId="" UserId2="" StatPer="0"/>' \
            'https://authdl.my.games/gem.php?hint=Auth')

        # New EULA Required
        if echo "$res" | grep -- 'ErrorCode="505"'; then
            res=$(curl -s \
                -A "$ua" \
                -b $cookie -c $cookie \
                'https://api.my.games/social/profile/session')

            csrfmiddlewaretoken_jwt=$(echo "$res" | sed 's/^.*"token":"\([^"]*\).*$/\1/')
            [ -z "$csrfmiddlewaretoken_jwt" ] && error 5
            res=$(curl -s \
                -A "$ua" \
                --data-urlencode "csrfmiddlewaretoken_jwt=${csrfmiddlewaretoken_jwt}" \
                --data-urlencode "csrfmiddlewaretoken=" \
                -b $cookie -c $cookie \
                'https://api.my.games/account/terms_accept/')
            echo "$res" | grep '"OK"' || error 6
            $0 $@
            exit $?
        fi

        rm -f $cookie

        echo "$res" | grep -- 'ErrorCode' && error 1

        SessionKey=$(echo "$res" | sed 's/^.* SessionKey="\([^"]*\).*$/\1/')

        res=$(curl -s \
            -A "$ua" \
            -d '<?xml version="1.0" encoding="UTF-8"?><Portal SessionKey="'"${SessionKey}"'" Url="http://authdl.my.games/robots.txt"/>' \
            'https://authdl.my.games/gem.php?hint=Portal')

        echo "$res" | grep -- 'ErrorCode' && error 1

        RedirectUrl=$(echo "$res" | sed 's/^.* RedirectUrl="\([^"]*\).*$/\1/' | sed 's/&amp;/\&/g')

        res=$(curl -Ls \
            -A "$ua" \
            "${RedirectUrl}")

        res=$(curl -s \
            -A "$ua" \
            -d '<?xml version="1.0" encoding="UTF-8"?><Login SessionKey="'"${SessionKey}"'" ProjectId="'"${ProjectId}"'" ShardId="'"${ShardId}"'"/>' \
            'https://authdl.my.games/gem.php?hint=Login')

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

        echo 'done'

        token=$(echo "$res" | sed 's/^.*access_token":"\([^"]*\).*$/\1/')

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

server="./cfg/server/${realm}.cfg"

if [ -z $WB_AS_DAEMON ]; then
    ${WB} -t ${token} -i ${userid} -f ${server} $@
else
    if [ -z $NO_FORK ]; then
        ${WBD} -t ${token} -i ${userid} -f ${server} $@ &
    else
        ${WB} -t ${token} -i ${userid} -f ${server} $@
    fi
fi

