#!/bin/sh
# prepare environment
mkdir -p /usr/lib/utempter
cp utempter /usr/lib/utempter/

# run test
./test_utempter

# clean environment
tmp_file="wtmp.txt"
utmpdump /var/log/wtmp > "${tmp_file}"
sed -i '/test_libutempter/,$d' "${tmp_file}"

# restore wtmp file
utmpdump -r < "${tmp_file}" > /var/log/wtmp
rm -rf "${tmp_file}" /usr/lib/utempter
