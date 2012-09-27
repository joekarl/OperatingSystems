#!/bin/env bash

./agent 30 561 566 562 &
./agent 60 562 561 563 &
./agent 60 563 562 564 &
./agent 45 564 563 565 &
./agent 100 565 564 566 &
./agent 2 566 565 561 &

