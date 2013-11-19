build-essential:
  pkg.installed

cmake:
  pkg.installed

clang:
  pkg.installed

libblocksruntime-dev:
  pkg.installed

libkqueue-dev:
  pkg.installed

libpthread-workqueue-dev:
  pkg.installed

gobjc:
  pkg.installed

libxml2-dev:
  pkg.installed

libjpeg-dev:
  pkg.installed

libtiff4-dev:
  pkg.installed

libpng12-dev:
  pkg.installed

libcups2-dev:
  pkg.installed

libfreetype6-dev:
  pkg.installed

libcairo2-dev:
  pkg.installed

libxt-dev:
  pkg.installed

libgl1-mesa-dev:
  pkg.installed

libicu-dev:
  pkg.installed

curl:
  pkg.installed

download-gnustep-make:
  cmd.run:
    - name: curl -o gnustep-make.tar.gz ftp://ftp.gnustep.org/pub/gnustep/core/gnustep-make-2.6.5.tar.gz
    - cwd: /tmp/
    - require:
      - pkg: curl

download-gnustep-back:
  cmd.run:
    - name: curl -o gnustep-back.tar.gz ftp://ftp.gnustep.org/pub/gnustep/core/gnustep-back-0.23.0.tar.gz
    - cwd: /tmp/
    - require:
      - pkg: curl

download-gnustep-gui:
  cmd.run:
    - name: curl -o gnustep-gui.tar.gz ftp://ftp.gnustep.org/pub/gnustep/core/gnustep-gui-0.23.1.tar.gz
    - cwd: /tmp/
    - require:
      - pkg: curl

download-gnustep-base:
  cmd.run:
    - name: curl -o gnustep-base.tar.gz ftp://ftp.gnustep.org/pub/gnustep/core/gnustep-base-1.24.5.tar.gz
    - cwd: /tmp/
    - require:
      - pkg: curl

download-gnustep-corebase:
  cmd.run:
    - name: curl -o gnustep-corebase.tar.gz ftp://ftp.gnustep.org/pub/gnustep/libs/gnustep-corebase-0.1.1.tar.gz
    - cwd: /tmp/
    - require:
      - pkg: curl

download-libobjc:
  cmd.run:
    - name: curl -o libobjc2-1.6.tar.gz http://download.gna.org/gnustep/libobjc2-1.6.tar.gz
    - cwd: /tmp/
    - require:
      - pkg: curl

extract-libobjc:
  cmd.run:
    - name: tar zxf libobjc2-1.6.tar.gz
    - cwd: /tmp/
    - require:
      - cmd.run: download-libobjc

extract-gnustep-make:
  cmd.run:
    - name: tar zxf gnustep-make.tar.gz
    - cwd: /tmp/
    - require:
      - cmd.run: download-gnustep-make

extract-gnustep-base:
  cmd.run:
    - name: tar zxf gnustep-base.tar.gz
    - cwd: /tmp/
    - require:
      - cmd.run: download-gnustep-base

extract-gnustep-corebase:
  cmd.run:
    - name: tar zxf gnustep-corebase.tar.gz
    - cwd: /tmp/
    - require:
      - cmd.run: download-gnustep-corebase

extract-gnustep-gui:
  cmd.run:
    - name: tar zxf gnustep-gui.tar.gz
    - cwd: /tmp/
    - require:
      - cmd.run: download-gnustep-gui

extract-gnustep-back:
  cmd.run:
    - name: tar zxf gnustep-back.tar.gz
    - cwd: /tmp/
    - require:
      - cmd.run: download-gnustep-back

#install-libobjc:
#  cmd.run:
#    - name: mkdir -p Build && cd Build && cmake .. -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ && make && make install
#    - cwd: /tmp/libobjc2-1.7/
#    - require:
#      - cmd.run: extract-libobjc

install-libobjc:
  cmd.run:
    - name: export CC=clang && make && make install
    - cwd: /tmp/libobjc2-1.6/
    - require:
      - cmd.run: extract-libobjc

install-gnustep-make:
  cmd.run:
    - name: export CC=clang && ./configure && make install
    - cwd: /tmp/gnustep-make-2.6.5/
    - require:
      - cmd.run: install-libobjc

install-gnustep-base:
  cmd.run:
    - name: export CC=clang && ./configure && make install
    - cwd: /tmp/gnustep-base-1.24.5/
    - require:
      - cmd.run: install-gnustep-make

install-gnustep-corebase:
  cmd.run:
    - name: export CC=clang && ./configure && make install
    - cwd: /tmp/gnustep-corebase-0.1.1/
    - require:
      - cmd.run: install-gnustep-base

install-gnustep-gui:
  cmd.run:
    - name: ldconfig && export CC=clang && ./configure && make install
    - cwd: /tmp/gnustep-gui-0.23.1/
    - require:
      - cmd.run: install-gnustep-corebase

install-gnustep-back:
  cmd.run:
    - name: export CC=clang && ./configure && make install
    - cwd: /tmp/gnustep-back-0.23.0/
    - require:
      - cmd.run: install-gnustep-gui

libdispatch-dev:
  pkg.installed:
  - require:
    - cmd.run: install-gnustep-back
