#!/bin/bash
echo "Compiling ETJump and setting up the server";
sudo mount -t vboxsf /home/etjump/github /home/etjump/github;
cd /home/etjump/github/etjump/src/;
scons;
/home/etjump/et/etded +set fs_game "etjump" +map oasis