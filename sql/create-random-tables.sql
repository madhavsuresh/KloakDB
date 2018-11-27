select setseed(.0001);

insert into left_deep_joins_1024 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,1024) s(i);
insert into left_deep_joins_4096 (a,b) select floor(random()*10000+1)::int, floor(random()*2000+1)::int from generate_series(1,4096) s(i);

insert into left_deep_joins_25200 (a,b) select floor(random()*10000+1)::int, floor(random()*100+1)::int from generate_series(1,25200) s(i);

insert into oblivious_partitioning_100 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,100) s(i);


insert into oblivious_partitioning_1000 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,1000) s(i);

insert into oblivious_partitioning_10000 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,10000) s(i);

insert into oblivious_partitioning_100000 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,100000) s(i);

insert into oblivious_partitioning_1000000 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,1000000) s(i);

insert into oblivious_partitioning_10000000 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,10000000) s(i);
