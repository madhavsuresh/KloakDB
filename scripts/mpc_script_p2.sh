cd ../build

for i in `seq 1 7`;
do
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 5 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 10 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 15 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 20 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 30 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 40 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 50 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 60 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 70 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 80 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 90 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 100 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 200 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 300 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2 -gen 350 -host_short=v2 -hostname="129.105.61.200"
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 2  -obli=true -host_short=v2 -hostname="129.105.61.200"
done

