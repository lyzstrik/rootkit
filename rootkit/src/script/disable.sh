#!/bin/bash

BASHRC="/home/kit/.bashrc"
PERSONALITY_CMD="/home/kit/knock/rootkit/bin/personality"

sed -i "\|$PERSONALITY_CMD|d" "$BASHRC"
