#!/bin/bash -x


# test a single request (passed)
time curl -v --proxy http://127.0.0.1:5555 http://info.cern.ch/

