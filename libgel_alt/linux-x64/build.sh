#!/usr/bin/env bash
vagrant up
vagrant ssh
cmake /vagrant
sudo make -j4
cp Core /vagrant/Core
