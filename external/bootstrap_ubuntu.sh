#!/usr/bin/env bash
sudo apt-get update
sudo apt-get -y install cmake g++ libglew-dev libsdl2-dev libfreetype6-dev
mkdir /vagrant/build && cd /vagrant/build && cmake .. && make


