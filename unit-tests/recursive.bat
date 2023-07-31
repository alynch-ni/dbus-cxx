start test-recursive B

test-recursive A
set CLIENT_ERRORLEVEL=%ERRORLEVEL%

ping /n 2 127.0.0.1 > NUL

EXIT /B %CLIENT_ERRORLEVEL%
