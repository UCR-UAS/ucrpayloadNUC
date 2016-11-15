#!/bin/bash

kill -9 `ps aux | grep "python ./Cap" | awk '{print $2}'`

kill -9 `ps aux | grep "SpectralSaliency" | awk '{print $2}'`
