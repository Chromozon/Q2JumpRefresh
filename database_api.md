# Jump Database Service API

## addtime
        {
            "login_token": "<server secret login token>" (string),
            "command": "addtime",
            "command_args": {
                "mapname": "ddrace" (string, no file extension),
                "username": "Slip" (string),
                "date": 1610596223836 (int, Unix time s),
                "time_ms": 8876 (int, completion time in server milliseconds),
                "pmove_time_ms": 8850 (int, -1 means no time, completion time sum of pmove packets),
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
### Returns:
        {
            "mapname": "ddrace" (string),
            "page": 1 (int, 1-based),
            "max_pages": 20 (int, total number of pages available),
            "count_per_page": 20 (int, how many results are returned in user_records),
            "user_count": 1023 (int, total number of users),
            "last_updated": 1610596223836 (int, Unix time s),
            "user_records":
            [
                "rank": 1 (int, 1-based, overall rank),
                "username": "Slip" (string),
                "server_name_short": "Ger1" (string),
                "date": 1610596223836 (int, Unix time s),
                "time_ms": 8876 (int, completion time in server milliseconds),
                "pmove_time_ms": 8850 (int, -1 means no time, completion time sum of pmove packets)
            ],
            [
                ...
            ]
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
### Returns:
        {
            "page": 1 (int, 1-based),
            "max_pages": 70 (int, total number of pages available),
            "count_per_page": 15 (int, how many results are returned in user_records),
            "user_count": 1023 (int, total number of users),
            "last_updated": 1610596223836 (int, Unix time s),
            "user_records":
            [
                "rank": 1 (int, 1-based, overall rank),
                "username": "Slip" (string),
                "highscore_counts": "12,10,8,4,5,0,0,0,2,0,0,0,1,0,0" (string, count of top15 times),
                "total_score": 5400 (int, overall score of all top15 times)
            ],
            [
                ...
            ]
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
### Returns:
        {
            "page": 1 (int, 1-based),
            "max_pages": 70 (int, total number of pages available),
            "count_per_page": 15 (int, how many results are returned in user_records),
            "user_count": 1023 (int, total number of users),
            "last_updated": 1610596223836 (int, Unix time s),
            "user_records":
            [
                "rank": 1 (int, 1-based, overall rank),
                "username": "Slip" (string),
                "highscore_counts": "12,10,8,4,5,0,0,0,2,0,0,0,1,0,0" (string, count of top15 times),
                "percent_score": 78.2 (float, percentage score, calculated as if user got first place in all maps)
            ],
            [
                ...
            ]
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
### Returns:
        {
            "page": 1 (int, 1-based),
            "max_pages": 70 (int, total number of pages available),
            "count_per_page": 15 (int, how many results are returned in user_records),
            "user_count": 1023 (int, total number of users),
            "last_updated": 1610596223836 (int, Unix time s),
            "user_records":
            [
                "rank": 1 (int, 1-based, overall rank),
                "username": "Slip" (string),
                "completions": 2600 (int, total number of maps completed),
                "percent_complete": 92.6 (float, percent completed of total maps)
            ],
            [
                ...
            ]
        }
