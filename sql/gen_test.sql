---RANGE 1000
DROP TABLE IF EXISTS gen_test_r1000_s1000_1, gen_test_r1000_s10000_1, gen_test_r1000_s100000_1, gen_test_r1000_s1000_2, gen_test_r1000_s10000_2, gen_test_r1000_s100000_2;
CREATE TABLE gen_test_r1000_s1000_1(
	a INT,
	b INT
);
CREATE TABLE gen_test_r1000_s10000_1(
	a INT,
	b INT
);
CREATE TABLE gen_test_r1000_s100000_1(
	a INT,
	b INT
);
CREATE TABLE gen_test_r1000_s1000_2(
	a INT,
	b INT
);
CREATE TABLE gen_test_r1000_s10000_2(
	a INT,
	b INT
);
CREATE TABLE gen_test_r1000_s100000_2(
	a INT,
	b INT
);

insert into gen_test_r1000_s1000_1 (a,b) (SELECT floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,1000) s(i));
insert into gen_test_r1000_s10000_1 (a,b) (select floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,10000) s(i));
insert into gen_test_r1000_s100000_1 (a,b) select floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,100000) s(i);
insert into gen_test_r1000_s1000_2 (a,b) (SELECT floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,1000) s(i));
insert into gen_test_r1000_s10000_2 (a,b) (select floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,10000) s(i));
insert into gen_test_r1000_s100000_2 (a,b) select floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,100000) s(i);

---RANGE 10000
DROP TABLE IF EXISTS gen_test_r10000_s1000_1, gen_test_r10000_s10000_1, gen_test_r10000_s100000_1, gen_test_r10000_s1000_2, gen_test_r10000_s10000_2, gen_test_r10000_s100000_2;
CREATE TABLE gen_test_r10000_s1000_1(
	a INT,
	b INT
);
CREATE TABLE gen_test_r10000_s10000_1(
	a INT,
	b INT
);
CREATE TABLE gen_test_r10000_s100000_1(
	a INT,
	b INT
);
CREATE TABLE gen_test_r10000_s1000_2(
	a INT,
	b INT
);
CREATE TABLE gen_test_r10000_s10000_2(
	a INT,
	b INT
);
CREATE TABLE gen_test_r10000_s100000_2(
	a INT,
	b INT
);

insert into gen_test_r10000_s1000_1 (a,b) (SELECT floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,1000) s(i));
insert into gen_test_r10000_s10000_1 (a,b) (select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,10000) s(i));
insert into gen_test_r10000_s100000_1 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,100000) s(i);
insert into gen_test_r10000_s1000_2 (a,b) (SELECT floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,1000) s(i));
insert into gen_test_r10000_s10000_2 (a,b) (select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,10000) s(i));
insert into gen_test_r10000_s100000_2 (a,b) select floor(random()*10000+1)::int, floor(random()*10000+1)::int from generate_series(1,100000) s(i);

---RANGE 100000
DROP TABLE IF EXISTS gen_test_r100000_s1000_1, gen_test_r100000_s10000_1, gen_test_r100000_s100000_1, gen_test_r100000_s1000_2, gen_test_r100000_s10000_2, gen_test_r100000_s100000_2;
CREATE TABLE gen_test_r100000_s1000_1(
	a INT,
	b INT
);
CREATE TABLE gen_test_r100000_s10000_1(
	a INT,
	b INT
);
CREATE TABLE gen_test_r100000_s100000_1(
	a INT,
	b INT
);
CREATE TABLE gen_test_r100000_s1000_2(
	a INT,
	b INT
);
CREATE TABLE gen_test_r100000_s10000_2(
	a INT,
	b INT
);
CREATE TABLE gen_test_r100000_s100000_2(
	a INT,
	b INT
);

insert into gen_test_r100000_s1000_1 (a,b) (SELECT floor(random()*10000+1)::int, floor(random()*100000+1)::int from generate_series(1,1000) s(i));
insert into gen_test_r100000_s10000_1 (a,b) (select floor(random()*10000+1)::int, floor(random()*100000+1)::int from generate_series(1,10000) s(i));
insert into gen_test_r100000_s100000_1 (a,b) select floor(random()*10000+1)::int, floor(random()*100000+1)::int from generate_series(1,100000) s(i);
insert into gen_test_r100000_s1000_2 (a,b) (SELECT floor(random()*10000+1)::int, floor(random()*100000+1)::int from generate_series(1,1000) s(i));
insert into gen_test_r100000_s10000_2 (a,b) (select floor(random()*10000+1)::int, floor(random()*100000+1)::int from generate_series(1,10000) s(i));
insert into gen_test_r100000_s100000_2 (a,b) select floor(random()*10000+1)::int, floor(random()*100000+1)::int from generate_series(1,100000) s(i);
