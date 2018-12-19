select setseed(.0001);

insert into left_deep_joins_512 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,512) s(i);
insert into left_deep_joins_1024 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,1024) s(i);
insert into left_deep_joins_4096 (a,b) select floor(random()*10000+1)::int, floor(random()*2000+1)::int from generate_series(1,4096) s(i);

insert into left_deep_joins_25200 (a,b) select floor(random()*10000+1)::int, floor(random()*100+1)::int from generate_series(1,25200) s(i);

insert into oblivious_partitioning_100 (a,b) select floor(random()*1000+1)::int, floor(random()*1000+1)::int from generate_series(1,100) s(i);


insert into oblivious_partitioning_1000 (a,b) select floor(random()*1000+1)::int, floor(random()*1000+1)::int from generate_series(1,1000) s(i);

insert into oblivious_partitioning_10000 (a,b) select floor(random()*1000+1)::int, floor(random()*1000+1)::int from generate_series(1,10000) s(i);

insert into oblivious_partitioning_100000 (a,b) select floor(random()*1000+1)::int, floor(random()*1000+1)::int from generate_series(1,100000) s(i);

insert into oblivious_partitioning_1000000 (a,b) select floor(random()*1000+1)::int, floor(random()*1000000+1)::int from generate_series(1,1000000) s(i);

insert into cdiff_cohort_diagnoses (major_icd9) select floor(random()*10000+1)::int from generate_series(1,100000) s(i);

insert into gen_test_a_h1 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_a_h2 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_a_h3 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_a_h4 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_a_h1_c (a,b) (select * from gen_test_a_h1);
insert into gen_test_a_h2_c (a,b) (select * from gen_test_a_h2);
insert into gen_test_a_h3_c (a,b) (select * from gen_test_a_h3);
insert into gen_test_a_h4_c (a,b) (select * from gen_test_a_h4);

insert into gen_test_b_h1 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_b_h2 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_b_h3 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_b_h4 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);

insert into gen_test_c_h1 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_c_h2 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_c_h3 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_c_h4 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);

insert into gen_test_d_h1 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_d_h2 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_d_h3 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
insert into gen_test_d_h4 (a,b) select floor(random()*10000+1)::int, floor(random()*1800000+1)::int from generate_series(1,180000) s(i);
