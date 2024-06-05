#!/bin/bash -x


# test cache upsert (passed)
time curl -s -o /dev/null -w "http response code: %{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDev5.html &
time curl -s -o /dev/null -w "http response code: %{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDev5.html &

sleep 15

# test cache retrieval (passed)
time curl -s -o /dev/null -w "http response code: %{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDev5.html


# test cache expiry (passed)
#sleep 20m
#time curl -s -o /dev/null -w "http response code: %{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDev5.html

