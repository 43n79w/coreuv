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

post-install-libuv:
  cmd.run:
    - name: install -m 644 /tmp/libuv-v0.10.18/out/Debug/libuv.a /usr/local/lib; install -m 644 /tmp/libuv-v0.10.18/include/uv.h /usr/local/include; install -m 755 -d /usr/local/include/uv-private; install -m 644 /tmp/libuv-v0.10.18/include/uv-private/stdint-msvc2008.h /usr/local/include/uv-private; install -m 644 /tmp/libuv-v0.10.18/include/uv-private/tree.h /usr/local/include/uv-private; install -m 644 /tmp/libuv-v0.10.18/include/uv-private/uv-bsd.h /usr/local/include/uv-private; install -m 644 /tmp/libuv-v0.10.18/include/uv-private/uv-darwin.h /usr/local/include/uv-private; install -m 644 /tmp/libuv-v0.10.18/include/uv-private/uv-linux.h /usr/local/include/uv-private; install -m 644 /tmp/libuv-v0.10.18/include/uv-private/uv-sunos.h /usr/local/include/uv-private; install -m 644 /tmp/libuv-v0.10.18/include/uv-private/uv-unix.h /usr/local/include/uv-private; install -m 644 /tmp/libuv-v0.10.18/include/uv-private/uv-win.h /usr/local/include/uv-private; install -m 644 /tmp/libuv-v0.10.18/include/uv-private/ngx-queue.h /usr/local/include/uv-private
    - cwd: /tmp/libuv-v0.10.18/
    - require:
      - cmd.run: install-libuv
