libyajl-dev:
  pkg.installed

libtidy-dev:
  pkg.installed

libcurl4-openssl-dev:
  pkg.installed

python2.7:
  pkg.installed

subversion:
  pkg.installed

download-libuv:
  cmd.run:
    - name: curl -o libuv-v0.10.18.tar.gz http://libuv.org/dist/v0.10.18/libuv-v0.10.18.tar.gz
    - cwd: /tmp/
    - require:
      - pkg: python2.7

extract-libuv:
  cmd.run:
    - name: tar zxf libuv-v0.10.18.tar.gz
    - cwd: /tmp/
    - require:
      - cmd.run: download-libuv

install-libuv:
  cmd.run:
    - name: mkdir -p build && svn co http://gyp.googlecode.com/svn/trunk build/gyp && ./gyp_uv -f make && make -C out
    - cwd: /tmp/libuv-v0.10.18/
    - require:
      - cmd.run: extract-libuv

