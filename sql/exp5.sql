select setseed(.08);
DROP TABLE IF EXISTS left_deep_joins_512_a, left_deep_joins_512_b, left_deep_joins_512_c, left_deep_joins_512_d,left_deep_joins_512_e;

CREATE TABLE left_deep_joins_512_a (
	a INT,
	b INT
);

CREATE TABLE left_deep_joins_512_b (
	a INT,
	b INT
);

CREATE TABLE left_deep_joins_512_c (
	a INT,
	b INT
);

CREATE TABLE left_deep_joins_512_d (
	a INT,
	b INT
);
CREATE TABLE left_deep_joins_512_e (
	a INT,
	b INT
);

insert into left_deep_joins_512_a (a,b) select floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,256) s(i);
insert into left_deep_joins_512_b (a,b) select floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,256) s(i);
insert into left_deep_joins_512_c (a,b) select floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,256) s(i);
insert into left_deep_joins_512_d (a,b) select floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,256) s(i);
insert into left_deep_joins_512_e (a,b) select floor(random()*10000+1)::int, floor(random()*1000+1)::int from generate_series(1,256) s(i);
