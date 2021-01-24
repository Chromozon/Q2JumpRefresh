# Jump Database Service API

## addtime
        {
            "login_token": "<server secret login token>" (string),
            "command": "addtime",
            "command_args": {
                "mapname": "ddrace" (string, no file extension),
                "username": "Slip" (string),
                "date": 1610596223836 (int, Unix time s),
                "time_ms": 234937 (int, ms),
                "pmove_time_ms": 223320 (int, ms, -1 means no time),
                "replay_data": "dGhpcyBpcyBhIHRlc3Q=" (string, base64 encoded replay frames)
            }
        }

## userlogin
        {
            "login_token": "<server secret login token>" (string),
            "command": "userlogin",
            "command_args": {
                "username": "Slip" (string),
                "password": "123456789" (string)
            }
        }

## changepassword
        {
            "login_token": "<server secret login token>" (string),
            "command": "changepassword",
            "command_args": {
                "username": "Slip" (string),
                "password_old": "123456789" (string),
                "password_new": "abc123" (string)
            }
        }

## addmap
        {
            "login_token": "<server secret login token>" (string),
            "command": "addmap",
            "command_args": {
                "mapname": "ddrace" (string, no file extension)
            }
        }

## maptimes
        {
            "login_token": "<server secret login token>" (string),
            "command": "maptimes",
            "command_args": {
                "mapname": "ddrace" (string, no file extension),
                "page": 1 (int, 1-based),
                "count_per_page": 15 (int, how many results to return)
            }
        }

## playertimes
        {
            "login_token": "<server secret login token>" (string),
            "command": "playertimes",
            "command_args": {
                "page": 1 (int, 1-based),
                "count_per_page": 20 (int, how many results to return)
            }
        }

## playerscores
        {
            "login_token": "<server secret login token>" (string),
            "command": "playerscores",
            "command_args": {
                "page": 1 (int, 1-based),
                "count_per_page": 20 (int, how many results to return)
            }
        }

## playermaps
        {
            "login_token": "<server secret login token>" (string),
            "command": "playermaps",
            "command_args": {
                "page": 1 (int, 1-based),
                "count_per_page": 20 (int, how many results to return)
            }
        }
