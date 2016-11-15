#!/bin/bash

tar -czf images.tar done

scp images.tar rachel@192.168.50.156:~/

rm -fr images.tar
