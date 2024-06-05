#!/bin/bash -x


# stress-testing (passed)
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://www.neverssl.com/ &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://info.cern.ch/hypertext/WWW/TheProject.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://info.cern.ch/ &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDevIndex.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://www.neverssl.com/ &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://info.cern.ch/ &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://info.cern.ch/hypertext/WWW/TheProject.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDev3.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://info.cern.ch/ &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://www.neverssl.com/ &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://info.cern.ch/ &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDev3.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDev5.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://www.neverssl.com/ &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://info.cern.ch/hypertext/WWW/TheProject.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://info.cern.ch/ &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDevIndex.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://brokenthorn.com/Resources/OSDev5.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://info.cern.ch/hypertext/WWW/TheProject.html &
curl -s -o /dev/null -w "%{http_code}\n" --proxy http://127.0.0.1:5555 http://www.neverssl.com/ &

