# Jump Database Service API

## addtime
{
    "login_token": "<server secret login token>" (string),
    "command": "addtime",
    "command_args": {
        "mapname": "mapname" (string, no file extension),
        "username": "username" (string),
        "date": 1610596223836 (int, Unix time s),
        "time_ms": 234937 (int, ms),
        "pmove_time_ms": 223320 (int, ms, -1 means no time),
        "replay_data": "dGhpcyBpcyBhIHRlc3Q=" (string, base64 encoded)
    }
}

## userlogin

## changepassword

## addmap

## playertimes

## playerscores

## playermaps

## maptimes