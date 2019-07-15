cd ../build

for i in `seq 1 7`;
do
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 5 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 10 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 15 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 20 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 30 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 40 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 50 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 60 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 70 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 80 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 90 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 100 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 200 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 300 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1 -gen 350 -host_short=v1
    LD_LIBRARY_PATH=../lib/lib/ ./bin/left_deep_lib -party 1  -obli=true -host_short=v1
done

