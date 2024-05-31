if [ "$#" -ne 4 ]; then
   echo "Usage: $0 janus-version major-version minor-version build-version"
   exit 1
fi

rm -rf workapps-output
rm -rf /opt/janus
mkdir -p /opt/janus
sh autogen.sh
./configure --disable-all-plugins --enable-plugin-videoroom --disable-all-handlers --enable-sample-event-handler --disable-all-transports --enable-websockets --enable-rest --enable-post-processing  --prefix=/opt/janus
make clean
make
make install
make configs

cp -rf support/supporting_files /opt/janus
cp -f support/install.sh /opt/janus
cp -f support/supporting_libs.tar.gz /opt/janus

version="janus_v"$1"_w"$2"."$3"-"$4-CentOS7.7-x86_64.tar.gz
echo "final artifact name is ${version}"
echo $version > /opt/janus/version

cd /opt/janus
tar -xhzvf /opt/janus/supporting_libs.tar.gz
tar -czvf ${version} bin include etc lib share supporting_files supporting_libs install.sh version
cd -
mkdir -p workapps-output
echo "copying final file from:/opt/janus/${version} to:`pwd`/workapps-output"
cp -f /opt/janus/${version} `pwd`/workapps-output