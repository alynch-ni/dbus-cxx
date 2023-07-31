start introspection-tests server %1
ping /n 2 127.0.0.1 > NUL

introspection-tests client %1
set CLIENT_ERRORLEVEL=%ERRORLEVEL%

ping /n 2 127.0.0.1 > NUL

EXIT /B %CLIENT_ERRORLEVEL%
