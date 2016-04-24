
# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure(2) do |config|
  config.vm.box = "ubuntu/trusty64"

  config.vm.provider "virtualbox" do |vb|
  #   # Display the VirtualBox GUI when booting the machine
  #   vb.gui = true
  #
  #   # Customize the amount of memory on the VM:
    vb.memory = "2048"
  end

  config.vm.provision "shell", inline: <<-SHELL
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test -y
    sudo apt-get update
    sudo apt-get install -y build-essential
    sudo apt-get install -y vim
    sudo apt-get install -y emacs
    sudo apt-get install -y libboost-all-dev
    sudo apt-get install -y unzip
    sudo apt-get install -y zip
    sudo apt-get install -y cmake
    sudo apt-get install -y g++-4.9
    

  SHELL
end